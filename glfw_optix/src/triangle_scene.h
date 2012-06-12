#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Render/PBO.h>
#include <Scene/proto_camera.h>
#include <Scene/OptixMesh.h>
#include <Scene/Mesh.h>
#include <File/MeshLoader.h>

#include "commonStructs.h"

struct OptixGeometryAndTriMesh_t
{
	optix::Geometry rtGeo;
	Scene::MeshPtr triMesh;
};

class OptixTriangleGeometry
{
public:
	static OptixGeometryAndTriMesh_t fromMeshData(Scene::MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir)
	{
		auto mesh = Scene::MeshPtr( new Scene::Mesh(data) );

		int num_indices = data->indices.size();
		int num_triangles = data->indices.size() / 3;
		int num_vertices = data->vertices.size() / 3;
		int num_normals = data->normals.size() / 3;

		optix::Geometry rtModel = rtContext->createGeometry();
		rtModel->setPrimitiveCount( num_triangles );
		optix::Program isect_program = rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_intersect" );
		optix::Program bbox_program = rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_bounds" );

		rtModel->setIntersectionProgram( isect_program );
		rtModel->setBoundingBoxProgram( bbox_program );
	
		optix::Buffer vertex_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getVbo()->getHandle() );
		vertex_buffer->setFormat(RT_FORMAT_USER);
		vertex_buffer->setElementSize(3*sizeof(float));
		vertex_buffer->setSize(num_vertices + num_normals);
		rtModel["vertex_buffer"]->setBuffer(vertex_buffer);

		optix::Buffer index_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getIbo()->getHandle() );
		index_buffer->setFormat(RT_FORMAT_INT3);
		index_buffer->setSize( num_triangles );
		rtModel["index_buffer"]->setBuffer(index_buffer);

		rtModel["normal_offset"]->setInt( num_normals );

		OptixGeometryAndTriMesh_t out = {rtModel, mesh};
		return out;
	}
};

class OptixScene
{
public:
	struct CamVars
	{
		optix::Variable eyePos;
		optix::Variable u;
		optix::Variable v;
		optix::Variable w;
	} optixCamVars;

	~OptixScene()
	{
		out_buffer_obj->unregisterGLBuffer();
		context->destroy();
	}

	OptixScene(std::string resource_dir, int width, int height) :
	  resource_dir(resource_dir), width(width), height(height)
	{
		optix_dir = resource_dir + "\\glfw_optix\\";
		init();
		createInstances();
	}

	void launch()
	{
		context->launch(0, width, height );
	}

	void compileScene()
	{
		context->validate();
		context->compile();
		context->launch(0,width,height);
	}

	optix::Variable getFTime()
	{
		return fTime;
	}

	optix::Buffer getOutBuffer()
	{
		return out_buffer_obj;
	}

	optix::Context getContext()
	{
		return context;
	}

	void moveCameraVertical(bool key_down, bool key_up, float deltaTime)
	{
		float dy = key_up - key_down;
		Scene::FirstPersonCamera::getSingleton()->move( glm::vec3(0.f, dy, 0.f), deltaTime );
	}

	void updateCamera(bool key_left, bool key_right, bool key_back, bool key_fwd, 
					 glm::vec2 mouse_coords, bool mouse_button_down, float deltaTime)
	{
		auto fps_camera = Scene::FirstPersonCamera::getSingleton();
		fps_camera->update( key_left, key_right, key_back, key_fwd, mouse_coords, mouse_button_down, deltaTime );

		static float time = 0.f;
		time += deltaTime;

		float aspect = width/(float)height;
		float vfov = fps_camera->getFovDegrees() * (3.14f/180.f);
		float screenDist = 1.0 / tan(vfov * 0.5);

		glm::vec3 eyePos = fps_camera->getPos();
		glm::vec3 u = aspect * fps_camera->getStrafeDirection();
		glm::vec3 v = fps_camera->getUpDirection();
		glm::vec3 w = screenDist * fps_camera->getLookDirection();
		
		optixCamVars.eyePos->set3fv( glm::value_ptr(eyePos) );
		optixCamVars.u->set3fv( glm::value_ptr(u) );
		optixCamVars.v->set3fv( glm::value_ptr(v) );
		optixCamVars.w->set3fv( glm::value_ptr(w) );
	}

	void renderRaster()
	{
		scene_instances[3]->render(nullptr);
		scene_instances[5]->render(nullptr);
		//for ( size_t i=0; i<scene_instances.size(); ++i ){
			//scene_instances[i]->render(nullptr);
		//}
	}

	void resize(int width, int height)
	{
		this->width = width;
		this->height = height;
		std::cout << "resizing buffers to " << width << " " << height << std::endl;
		
		// We don't want to allocate 0 memory for the PBOs
		width = width == 0 ? 1 : width; 
		height = height == 0 ? 1 : height; 

		glViewport(0,0,width,height);
		Scene::FirstPersonCamera::getSingleton()->updateProjection(width, height, 75.f, 0.01f, 1000.f);

		out_buffer_obj->setSize(width,height);

		try {
			// resize PBOs
			out_buffer_obj->unregisterGLBuffer();
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, out_buffer_obj->getGLBOId() );
			glBufferData(GL_PIXEL_UNPACK_BUFFER, out_buffer_obj->getElementSize() * width * height, 0, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			out_buffer_obj->registerGLBuffer();

		} catch ( optix::Exception& e ) {
			std::cout << e.what() << std::endl;
			system("pause");
			exit(-1);
		}


	}

	void animate()
	{
		//float t = fTime->getFloat();
		// remember to recalc accel when moving objects
		//top_level_acceleration->markDirty();
	}

private:
	void init()
	{
		context = optix::Context::create();
		context->setRayTypeCount(2); /* shadow and radiance */
		context->setEntryPointCount(1);
		context->setStackSize(2048);
		out_buffer_var = context->declareVariable("output_buffer");
		optix::Variable light_buffer = context->declareVariable("lights");
		context->declareVariable("max_depth")->setInt(3);
		context->declareVariable("radiance_ray_type")->setUint(0u);
		context->declareVariable("shadow_ray_type")->setUint(1u);
		context->declareVariable("scene_epsilon")->setFloat(1e-4f);

		/* Lights buffer */
		BasicLight light;
		light.color.x = 0.9f;
		light.color.y = 0.9f;
		light.color.z = 0.9f;
		light.pos.x   = 0.0f;
		light.pos.y   = 20.0f;
		light.pos.z   = 20.0f;
		light.casts_shadow = 1;
		light.padding      = 0u;

		optix::Buffer light_buffer_obj = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER, sizeof(BasicLight) );
		light_buffer_obj->setElementSize( sizeof(BasicLight) );
		light_buffer_obj->setSize(1);
		void* light_buffer_data = light_buffer_obj->map();
		((BasicLight*)light_buffer_data)[0] = light;
		light_buffer_obj->unmap();
		light_buffer->set(light_buffer_obj);

		lights.push_back( light );

		/* Ray gen program */
		std::string path_to_ptx = optix_dir + "\\pinhole_camera.cu.ptx";
		optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "pinhole_camera" );
		optixCamVars.eyePos = ray_gen_program->declareVariable("eye"); 
		optixCamVars.u = ray_gen_program->declareVariable("U"); 
		optixCamVars.v = ray_gen_program->declareVariable("V");
		optixCamVars.w = ray_gen_program->declareVariable("W");

		fTime = ray_gen_program->declareVariable("fTime");
		fTime->setFloat( 0.f );
		context->setRayGenerationProgram(0, ray_gen_program);

		/* Miss program */
		path_to_ptx = optix_dir + "\\gradientbg.cu.ptx";
		optix::Program miss_program = context->createProgramFromPTXFile( path_to_ptx, "miss" );
		glm::vec3 scene_up(0.f, 1.f, 0.f);
		context->declareVariable("background_light")->setFloat( 1.f, 1.f, 1.f );
		context->declareVariable("background_dark")->setFloat( 0.3f, 0.3f, 0.8f );
		context->declareVariable("up")->set3fv(glm::value_ptr(scene_up));
		context->setMissProgram(0, miss_program );

		/* Create shared GL/CUDA PBO */
		int element_size = 4 * sizeof(char);
		pbo = new Render::PBO(element_size * width * height, GL_STREAM_DRAW, true);
		out_buffer_obj = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, pbo->getHandle() );
		out_buffer_obj->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
		out_buffer_obj->setSize(width,height);
		
		out_buffer_var->set(out_buffer_obj);

		auto fps_camera = Scene::FirstPersonCameraPtr( Scene::FirstPersonCamera::getSingleton() );
		fps_camera->lookAt( glm::vec3(15.0f, 15.0f, 15.0f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) );
		fps_camera->updateProjection(width, height, 75.f, 0.01f, 1000.f);
		fps_camera->setSpeed( 20.f );
	}

	void createInstances()
	{
		top_level_group = context->createGroup();
		optix::Variable top_object = context->declareVariable("top_object");
		top_object->set( top_level_group );
		top_level_acceleration = context->createAcceleration("Bvh", "Bvh");
		top_level_group->setAcceleration(top_level_acceleration);

		recieve_shadow_group = context->createGroup();
		recieve_shadow_group->setAcceleration( context->createAcceleration("Bvh", "Bvh") );
		optix::Variable top_shadower = context->declareVariable("top_shadower");
		top_shadower->set(recieve_shadow_group);
		addToTopLevel( recieve_shadow_group );

		optix::Material material = createMaterial(); // createNormalDebugMaterial
		
		optix::Geometry box = createBoxGeometry();
		createFloor(box, material);
		createBoxInstances(box, material);

		std::string model_dir = resource_dir + "models\\";
		File::MeshLoader mesh_loader(model_dir);

		auto geo_hin = OptixTriangleGeometry::fromMeshData(mesh_loader.loadMeshDataEasy("hin_logo.3ds"), context, optix_dir);
		auto geo_disc = OptixTriangleGeometry::fromMeshData(mesh_loader.loadMeshDataEasy("disc.obj"), context, optix_dir);
		auto geo_ico = OptixTriangleGeometry::fromMeshData(mesh_loader.loadMeshDataEasy("icosphere.3ds"), context, optix_dir);

		// Create a few logos in a circle
		for ( int i=0; i<12; i++ ) {
			float ang_degs = i/12.f * 360.f;
			// translate, then rotate
			glm::mat4 xform = glm::rotate(ang_degs, 0.f,1.f,0.f) * glm::translate(0.f, 0.f, 15.f);

			auto hin_logo_instance = Scene::OptixMeshPtr( new Scene::OptixMesh(geo_hin.triMesh, geo_hin.rtGeo, material) );
			hin_logo_instance->setPosition( glm::vec3(xform[3]) );
			hin_logo_instance->setOrientation( glm::quat_cast(xform) );
			scene_instances.push_back(hin_logo_instance);

			addToShadowGroup( hin_logo_instance->getTransform() );
		}
		
		auto disc_instance = Scene::OptixMeshPtr( new Scene::OptixMesh(geo_disc.triMesh, geo_disc.rtGeo, material) );
		scene_instances.push_back(disc_instance);
		addToShadowGroup( disc_instance->getTransform() );

		// create ico sphere for each light
		for (size_t i=0; i<lights.size(); ++i){
			auto ico_instance = Scene::OptixMeshPtr( new Scene::OptixMesh(geo_ico.triMesh, geo_ico.rtGeo, material) );
			ico_instance->setPosition( glm::vec3(lights[i].pos.x,lights[i].pos.y,lights[i].pos.z) );
			scene_instances.push_back(ico_instance);

			addToTopLevel( ico_instance->getTransform() );
		}

		top_level_acceleration->markDirty();
	}

	optix::Material createMaterial()
	{
		//std::string path_to_ptx = optix_dir + "\\phong.cu.ptx";
		std::string path_to_ptx = optix_dir + "\\tut3_shadows.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Program any_hit_program = context->createProgramFromPTXFile( path_to_ptx, "any_hit_shadow" );

		optix::Material mat = context->createMaterial();
		mat->setClosestHitProgram(0, closest_hit_program);
		mat->setAnyHitProgram(1, any_hit_program);
		return mat;
	}

	optix::Material createNormalDebugMaterial()
	{
		std::string path_to_ptx = optix_dir + "\\mat_normal.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Material mat = context->createMaterial();
		 
		// debug normals only uses closest hit.
		mat->setClosestHitProgram(0 /*radiance*/, closest_hit_program);
		return mat;
	}

	void setInstanceMaterialParams( optix::GeometryInstance instance, 
					glm::vec3 ambient = glm::vec3(0.2f, 0.2f,0.2f),
					glm::vec3 kd = glm::vec3(0.5f, 0.5f,0.5f), 
					glm::vec3 ks = glm::vec3(0.5f, 0.5f,0.5f), 
					glm::vec3 ka = glm::vec3(0.8f, 0.8f,0.8f), 					 
					glm::vec3 reflectivity = glm::vec3(0.8f, 0.8f,0.8f) ,  
					float shinyness = 10.f )
	{
		optix::Variable var_kd =             instance->declareVariable("Kd");
		optix::Variable var_ks =             instance->declareVariable("Ks");
		optix::Variable var_ka =             instance->declareVariable("Ka");
		optix::Variable var_expv =           instance->declareVariable("phong_exp");
		optix::Variable var_reflectivity =   instance->declareVariable("reflectivity");
		optix::Variable var_ambient =        instance->declareVariable("ambient_light_color");
		var_ambient->set3fv( glm::value_ptr(ambient) );
		var_kd->set3fv( glm::value_ptr(kd) );
		var_ks->set3fv( glm::value_ptr(ks) );
		var_ka->set3fv( glm::value_ptr(ka) );
		var_reflectivity->set3fv( glm::value_ptr(reflectivity) );
		var_expv->setFloat(shinyness);
	}

	void createFloor(optix::Geometry box, optix::Material material)
	{
		optix::GeometryInstance instance = context->createGeometryInstance();
		instance->setGeometry(box);
		instance->setMaterialCount(1);
		instance->setMaterial(0, material );
		setInstanceMaterialParams( instance, glm::vec3(0.2f,0.0f,0.0f) );

		optix::GeometryGroup geometrygroup = context->createGeometryGroup();
		geometrygroup->setChildCount(1);
		geometrygroup->setChild(0,instance);

		optix::Acceleration acceleration = context->createAcceleration("Bvh", "Bvh");
		geometrygroup->setAcceleration(acceleration);
		acceleration->markDirty();

		optix::Transform transform = context->createTransform();
		transform->setChild( geometrygroup );
		addToTopLevel( transform );
		
		glm::mat4 xform(1.f);
		xform = glm::translate( xform, 0.f, -15.f, 0.f );
		xform = glm::scale(xform, 200.f, 1.f, 200.f);
		xform = glm::transpose(xform);
		transform->setMatrix( 0, glm::value_ptr(xform), 0 );
		acceleration->markDirty();
	}

	template< typename T >
	inline 
	void addToTopLevel(T child) // optix::Transform
	{
		int count = top_level_group->getChildCount();
		top_level_group->setChildCount( count + 1 );
		top_level_group->setChild(count, child );
	}

	template< typename T >
	inline 
	void addToShadowGroup(T child) // optix::Transform
	{
		int count = recieve_shadow_group->getChildCount();
		recieve_shadow_group->setChildCount( count + 1 );
		recieve_shadow_group->setChild(count, child );
	}

	glm::vec3 getLorenzAttractorDelta( glm::vec3 &p, int num_points )
	{
		float r=28;
		float s=10; // "Prandtl number" fluid viscosity / thermal conductivity, Lorenz chose 10
		float b=8.0/3.0;

		float imax = (float)num_points;
		float f = imax/(imax/100.0f);

		glm::vec3 delta( s*(p.y - p.x), 
				         r*p.x - p.y - p.x*p.z, 
						 p.x*p.y - b*p.z);
		return delta/f;
	}

	void createBoxInstances(optix::Geometry box, optix::Material material)
	{
		static const int NUM_BOXES = 1024;
		transforms.resize(NUM_BOXES);

		glm::vec3 lorenz_pos(0.1f, 0.0f, 0.0f);

		/* create acceleration object for group and specify some build hints*/
		cube_acceleration = context->createAcceleration("Bvh", "Bvh");
		
		for ( int i = 0; i < NUM_BOXES; ++i )
		{
			/* Create this geometry instance */
			optix::GeometryInstance instance = context->createGeometryInstance();
			instance->setGeometry(box);
			instance->setMaterialCount(1);
			instance->setMaterial(0, material);
			float kd_slider = (float)i / (float)(NUM_BOXES-1);
			setInstanceMaterialParams( instance, glm::vec3(0.2f,0.2f,0.2f), glm::vec3(kd_slider, 0.0f, 1.0f-kd_slider)  );

			/* create group to hold instance transform */
			optix::GeometryGroup geometrygroup = context->createGeometryGroup();
			geometrygroup->setChildCount(1);
			geometrygroup->setChild(0,instance);
			geometrygroup->setAcceleration(cube_acceleration);

			glm::vec3 delta = getLorenzAttractorDelta( lorenz_pos, NUM_BOXES );
			lorenz_pos += delta;

			transforms[i] = context->createTransform();
			addToTopLevel(transforms[i]);
			transforms[i]->setChild( geometrygroup );
			
			glm::mat4 xform(1.0f);
			xform = glm::translate( xform, lorenz_pos + glm::vec3(-25.f,5.f,-100.f) );
			xform = xform * glm::mat4_cast(glm::normalize(glm::quat(1.f, delta)));
			xform = glm::scale(xform, glm::vec3(0.5f) );
			xform = glm::transpose(xform);
			transforms[i]->setMatrix( 0, glm::value_ptr(xform), 0 );
		}
		cube_acceleration->markDirty();
	}

	optix::Geometry createBoxGeometry()
	{
		float     box_min[3];
		float     box_max[3];
		box_min[0] = box_min[1] = box_min[2] = -0.5f;
		box_max[0] = box_max[1] = box_max[2] =  0.5f;
		optix::Geometry box_geo = context->createGeometry();
		box_geo->setPrimitiveCount(1u);

		std::string path_to_ptx = optix_dir + "\\box.cu.ptx";
		optix::Program box_bounding_box_program = context->createProgramFromPTXFile( path_to_ptx, "box_bounds" );
		optix::Program box_intersection_program = context->createProgramFromPTXFile( path_to_ptx, "box_intersect" );
		box_geo->setBoundingBoxProgram(box_bounding_box_program);
		box_geo->setIntersectionProgram(box_intersection_program);
		box_geo->declareVariable("boxmin")->set3fv(box_min);
		box_geo->declareVariable("boxmax")->set3fv(box_max);

		return box_geo;
	}

public:
	Render::PBO *pbo;
private:
	optix::Context                   context;
	optix::Buffer                    out_buffer_obj;
	optix::Variable                  out_buffer_var;
	optix::Variable                  fTime;
	
	std::vector<optix::Transform>    transforms;
	optix::Group                     top_level_group;
	optix::Acceleration              top_level_acceleration;
	optix::Group                     recieve_shadow_group;
	// having one shared accel improved perf,
	// having one shared group did not improved perf.
	optix::Acceleration              cube_acceleration; 
	
	std::vector<Scene::OptixMeshPtr> scene_instances;
	
	std::vector<BasicLight>          lights;
	std::string                      optix_dir;
	std::string                      resource_dir;
	int                              width;
	int                              height;
};
