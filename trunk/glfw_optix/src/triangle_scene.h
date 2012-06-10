#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Render/PBO.h>
#include <Scene/proto_camera.h>
#include <Scene/OptixMesh.h>
#include <File/MeshLoader.h>

#include "commonStructs.h"

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

	OptixScene(std::string resource_dir, int width, int height) :
	  resource_dir(resource_dir), width(width), height(height)
	{
		optix_dir = resource_dir + "\\glfw_optix\\";
		init();
		createInstances();
	}

	void init()
	{
		/* Context */
		context = optix::Context::create();
		context->setRayTypeCount(2); /* shadow and radiance */
		context->setEntryPointCount(1);
		context->setStackSize(2048);
		optix::Variable out_buffer = context->declareVariable("output_buffer");
		optix::Variable light_buffer = context->declareVariable("lights");
		context->declareVariable("max_depth")->setInt(10);
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

		/* Ray gen program */
		std::string path_to_ptx = optix_dir + "\\pinhole_camera.cu.ptx";
		optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "pinhole_camera" );
		optixCamVars.eyePos = ray_gen_program->declareVariable("eye"); 
		optixCamVars.u = ray_gen_program->declareVariable("U"); 
		optixCamVars.v = ray_gen_program->declareVariable("V");
		optixCamVars.w = ray_gen_program->declareVariable("W");

		unsigned int screenDims[] = {width,height};
		ray_gen_program->declareVariable("rtLaunchDim")->set2uiv(screenDims);
		fTime = ray_gen_program->declareVariable("fTime");
		fTime->setFloat( 0.f );
		context->setRayGenerationProgram(0, ray_gen_program);

		/* Miss program */
		path_to_ptx = optix_dir + "\\constantbg.cu.ptx";
		optix::Program miss_program = context->createProgramFromPTXFile( path_to_ptx, "miss" );
		glm::vec3 missColor(.3f, 0.1f, 0.2f);
		miss_program->declareVariable("bg_color")->set3fv(&missColor.x);
		context->setMissProgram(0, miss_program );

		/* Create shared GL/CUDA PBO */
		int element_size = 4 * sizeof(char);
		pbo = new Render::PBO(element_size * width * height, GL_STREAM_DRAW, true);
		out_buffer_obj = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, pbo->getHandle() );
		out_buffer_obj->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
		out_buffer_obj->setSize(width,height);
		
		out_buffer->set(out_buffer_obj);

		fps_camera.lookAt( glm::vec3(15.0f, 15.0f, 15.0f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f) );
	}

	optix::Material createMaterial()
	{
		/* Create our hit programs to be shared among all materials */
		std::string path_to_ptx = optix_dir + "\\phong.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Program any_hit_program = context->createProgramFromPTXFile( path_to_ptx, "any_hit_shadow" );

		optix::Material mat = context->createMaterial();
		mat->setClosestHitProgram(0, closest_hit_program);
		mat->setAnyHitProgram(1, any_hit_program);
		return mat;
	}

	optix::Material createNormalDebugMaterial()
	{
		/* Create our hit programs to be shared among all materials */
		std::string path_to_ptx = optix_dir + "\\mat_normal.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Material mat = context->createMaterial();
		 
		// debug normals only uses closest hit.
		
		mat->setClosestHitProgram(0 /*radiance*/, closest_hit_program);
		return mat;
	}

	void createInstances()
	{
		optix::Group top_level_group = context->createGroup();
		optix::Variable top_object = context->declareVariable("top_object");
		top_object->set( top_level_group );
		optix::Variable top_shadower = context->declareVariable("top_shadower");
		top_shadower->set(top_level_group);
		top_level_acceleration = context->createAcceleration("Bvh", "Bvh");
		top_level_group->setAcceleration(top_level_acceleration);

		optix::Material material = createMaterial(); // createNormalDebugMaterial
		//createBoxInstances(top_level_group, createBoxGeometry(), material );

		//createFloor(top_level_group, material);

		std::string model_dir = resource_dir + "models\\";
		File::MeshLoader mesh_loader(model_dir);
		// TODO destruction
		spacejet = std::shared_ptr<Scene::OptixMesh>( new Scene::OptixMesh(mesh_loader.loadMeshDataEasy("hin_logo.3ds"), context, optix_dir));
		//disc = std::shared_ptr<Scene::OptixMesh>( new Scene::OptixMesh(mesh_loader.loadMeshDataEasy("disc.obj"), context, optix_dir));
		//big_cube = std::shared_ptr<Scene::OptixMesh>( new Scene::OptixMesh(mesh_loader.loadMeshDataEasy("cube.obj"), context, optix_dir));

		createMeshInstances(top_level_group, spacejet->getGeometry(), material, 0 );
		//createMeshInstances(top_level_group, disc->getGeometry(), material, 1 );
		//createMeshInstances(top_level_group, big_cube->getGeometry(), material, 2 );
		
		/* mark acceleration as dirty */
		top_level_acceleration->markDirty();
	}

	void setInstanceMaterialParams( optix::GeometryInstance instance, glm::vec3 diffuse, glm::vec3 reflect, float shinyness )
	{
		/* Set variables to be consumed by material for this geometry instance */
		struct material_data_def
		{
			glm::vec3 ambient;
			glm::vec3 kd, ks, ka;
			glm::vec3 reflectivity;
			float expv;
		} material_data;
		material_data.kd = diffuse;
		material_data.ks = glm::vec3(0.5f, 0.5f, 0.5f);
		material_data.ka = glm::vec3(0.8f, 0.8f, 0.8f);
		material_data.reflectivity = reflect; //glm::vec3(0.8f, 0.8f, 0.8f);
		material_data.expv = shinyness; //10.0f;
		material_data.ambient = glm::vec3(0.2f, 0.2f, 0.2f);

		optix::Variable kd = instance->declareVariable("Kd");
		optix::Variable ks = instance->declareVariable("Ks");
		optix::Variable ka = instance->declareVariable("Ka");
		optix::Variable expv = instance->declareVariable("phong_exp");
		optix::Variable reflectivity = instance->declareVariable("reflectivity");
		optix::Variable ambient = instance->declareVariable("ambient_light_color");
		kd->set3fv( &material_data.kd.x );
		ks->set3fv( &material_data.ks.x );
		ka->set3fv( &material_data.ka.x );
		reflectivity->set3fv( &material_data.reflectivity.x );
		expv->setFloat( material_data.expv );
		ambient->set3fv( &material_data.ambient.x );
	}

	/*
	Create actuall geometry instance, set up accel structure, setup transform.
	this should go in optix mesh ctor.... 
	only need to create GEO once... have multiple instances with their on transforms...
	Might want to have a Mesh class and Instance class, or have 
	Mesh-class (actually instance, but store GEO in assetgr, but keeping concepts
	clean & clear is better. one data -> one Mesh -> many instances/scene nodes
	*/
	void createMeshInstances(optix::Group top_level_group, optix::Geometry mesh, optix::Material material, int material_setting)
	{
		optix::GeometryInstance instance = context->createGeometryInstance();
		instance->setGeometry(mesh);
		instance->setMaterialCount(1);
		instance->setMaterial(0, material);

		// jet disc cube
		if (material_setting == 0 ) {
			setInstanceMaterialParams( instance, glm::vec3(0.2f,0.2f,0.2f), glm::vec3(0.f,0.f,0.f), 0.0f );
		} else if ( material_setting == 1 ) {
			setInstanceMaterialParams( instance, glm::vec3(0.0f,0.0f,0.2f), glm::vec3(0.8f,0.8f,0.8f), 10.f );
		} else if (material_setting==2) {
			setInstanceMaterialParams( instance, glm::vec3(0.2f,0.0f,0.0f), glm::vec3(0.f), 0.0f );
		}

		/* create group to hold instance transform */
		optix::GeometryGroup geometrygroup = context->createGeometryGroup();
		geometrygroup->setChildCount(1);
		geometrygroup->setChild(0,instance);

		/* create acceleration object for group and specify some build hints*/
		//optix::Acceleration acceleration = context->createAcceleration("Sbvh", "BvhCompact"); // split
		optix::Acceleration acceleration = context->createAcceleration("Bvh", "Bvh"); // classic
		//optix::Acceleration acceleration = context->createAcceleration("MedianBvh", "Bvh"); // fast construct (dyn content)
		//optix::Acceleration acceleration = context->createAcceleration("Lbvh", "Bvh"); // HLBVH2 algorithm, fast GPU bvh, use when BVH dominates runtime
		//optix::Acceleration acceleration =  context->createAcceleration("TriangleKdTree", "KdTree"); // requires "vertex_buffer" and "index_buffer" .cu variables set
		//acceleration->setProperty( "vertex_buffer_name", "vertex_buffer" );
		//acceleration->setProperty( "index_buffer_name", "index_buffer" );
		//optix::Acceleration acceleration = context->createAcceleration("NoAccel", "NoAccel"); // very inefficient for anything but simple cases (ok if very few primitives)

		geometrygroup->setAcceleration(acceleration);
		acceleration->markDirty();

		optix::Transform mesh_xfrom = context->createTransform();
		mesh_xfrom->setChild( geometrygroup );
		
		glm::mat4 xform(1.f);
		//xform = glm::translate( xform, 0.f, 0.f, 0.f );
		//xform = glm::scale( xform, glm::vec3(1.f) );
		xform = glm::transpose( xform );
		
		mesh_xfrom->setMatrix( 0, glm::value_ptr(xform), 0 );

		int cnt = top_level_group->getChildCount()+1;
		top_level_group->setChildCount(cnt);
		top_level_group->setChild(cnt-1, mesh_xfrom );
	}

	void createFloor(optix::Group top_level_group, optix::Material material)
	{
		optix::GeometryInstance instance = context->createGeometryInstance();
		instance->setGeometry(createBoxGeometry());
		instance->setMaterialCount(1);
		instance->setMaterial(0, material );
		setInstanceMaterialParams( instance, glm::vec3(0.5f,0.1f,0.1f), glm::vec3(0.5f,0.5f,0.5f), 3.f );

		/* create group to hold instance transform */
		optix::GeometryGroup geometrygroup = context->createGeometryGroup();
		geometrygroup->setChildCount(1);
		geometrygroup->setChild(0,instance);

		/* create acceleration object for group and specify some build hints*/
		optix::Acceleration acceleration = context->createAcceleration("Bvh", "Bvh");
		geometrygroup->setAcceleration(acceleration);
		acceleration->markDirty();

		optix::Transform mesh_xfrom = context->createTransform();
		mesh_xfrom->setChild( geometrygroup );
		
		glm::mat4 xform(1.f);
		// TRS
		xform = glm::translate( xform, 0.f, 3.f, 0.f );
		xform = glm::scale(xform, 25.f, 1.f, 25.f);
		
		
		mesh_xfrom->setMatrix( 0, glm::value_ptr(xform), 0 );

		int cnt = top_level_group->getChildCount()+1;
		top_level_group->setChildCount(cnt);
		top_level_group->setChild(cnt-1, mesh_xfrom );
	}

	void createBoxInstances(optix::Group top_level_group, optix::Geometry box, optix::Material material)
	{
		static const int NUM_BOXES = 24;
		transforms.resize(NUM_BOXES);

		for ( int i = 0; i < NUM_BOXES; ++i )
		{
			/* Create this geometry instance */
			optix::GeometryInstance instance = context->createGeometryInstance();
			instance->setGeometry(box);
			instance->setMaterialCount(1);
			instance->setMaterial(0, material);
			
			float kd_slider = (float)i / (float)(NUM_BOXES-1);
			setInstanceMaterialParams( instance, glm::vec3(kd_slider, 0.0f, 1.0f-kd_slider), glm::vec3(0.2f,0.2f,0.2f), 1.f );

			/* create group to hold instance transform */
			optix::GeometryGroup geometrygroup = context->createGeometryGroup();
			geometrygroup->setChildCount(1);
			geometrygroup->setChild(0,instance);

			/* create acceleration object for group and specify some build hints*/
			optix::Acceleration acceleration = context->createAcceleration("Bvh", "Bvh");
			geometrygroup->setAcceleration(acceleration);
			acceleration->markDirty();

			transforms[i] = context->createTransform();
			transforms[i]->setChild( geometrygroup );
			float seperation = 1.1f;
			glm::mat4 xform = glm::transpose(glm::translate( glm::mat4(1.f), i*seperation - (NUM_BOXES-1)*seperation*.5f,  0.f, 0.f ) );
			transforms[i]->setMatrix( 0, glm::value_ptr(xform), 0 );

			/* Place these geometrygroups as children of the top level object */
			int cnt = top_level_group->getChildCount()+1;
			top_level_group->setChildCount(cnt);
			top_level_group->setChild(cnt-1, transforms[i] );
		}
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

	void updateCamera(bool key_left, bool key_right, bool key_back, bool key_fwd, 
					 glm::vec2 mouse_coords, bool mouse_button_down, float deltaTime)
	{
		fps_camera.update( key_left, key_right, key_back, key_fwd, mouse_coords, mouse_button_down, deltaTime );
		glm::vec3 eyePos = fps_camera.getPos();
		float aspect = width/(float)height;
		glm::vec3 u = aspect * fps_camera.getStrafeDirection();
		glm::vec3 v = fps_camera.getUpDirection();
		glm::vec3 w = 2.2f * fps_camera.getLookDirection();
		
		optixCamVars.eyePos->set3fv(&eyePos.x);
		optixCamVars.u->set3fv(&u.x);
		optixCamVars.v->set3fv(&v.x);
		optixCamVars.w->set3fv(&w.x);
	}

	void animate()
	{
		float t = fTime->getFloat();
		glm::mat4 identity(1.f);		
		int num = transforms.size();
		for (size_t i=0; i<transforms.size(); ++i){
			float a = 6.28f * i/(float)num;
			glm::vec3 pos( cos(a), sin(4.f*a+t)*0.25f, sin(a) );
			pos *= 5.f;

			glm::mat4 xform = glm::translate( identity, pos );
			xform = glm::transpose(xform);

			if ( i==0 ) {
				// T*R*S
				glm::mat4 xform(1.0f);
				xform = glm::translate(xform, glm::vec3(0.f, -3.f, 0.f) );
				xform = glm::scale(xform, 25.f, 1.f, 25.f);
				
				transforms[i]->setMatrix( true, glm::value_ptr(xform),  0 );
			} else {
				transforms[i]->setMatrix( false, glm::value_ptr(xform),  0 );
			}
		}

		//top_level_acceleration->markDirty();
	}

	~OptixScene()
	{
		out_buffer_obj->unregisterGLBuffer();
		context->destroy();
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
public:
	Render::PBO *pbo;
private:
	optix::Context context;
	optix::Buffer out_buffer_obj;
	optix::Variable fTime;
	std::vector<optix::Transform> transforms;
	optix::Acceleration top_level_acceleration;

	Scene::OptixMeshPtr spacejet;
	Scene::OptixMeshPtr disc;
	Scene::OptixMeshPtr	big_cube;
	Scene::FirstPersonCamera fps_camera;
	std::string optix_dir;
	std::string resource_dir;
	int width;
	int height;
};
