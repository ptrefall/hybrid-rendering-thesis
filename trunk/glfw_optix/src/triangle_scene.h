#include "Raytracer/OptixTriMeshLoader.h"

#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Render/PBO.h>
#include <Scene/proto_camera.h>
#include <Scene/OptixInstance.h>
#include <Scene/Mesh.h>
#include <File/MeshLoader.h>
#include <File/BARTLoader2.h>
#include <Scene/BARTMesh.h>
#include <File/AssetManager.h>
#include <Parser/INIParser.h>

#include "commonStructs.h"

#include <map>

struct light_t{
	glm::vec3 pos; // 12
	glm::vec3 color; // 12
	int casts_shadow; // 4
	int padding; // 4
	light_t(const glm::vec3 &pos, const glm::vec3 &color) 
		: pos(pos), color(color), casts_shadow(1), padding(0)
	{
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
		context->launch( 0, width, height);
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
		//for ( size_t i=0; i<scene_instances.size(); ++i ){
		//	scene_instances[i]->render( boring_shader );
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
		light_buffer = context->declareVariable("lights");
		context->declareVariable("max_depth")->setInt(3);
		context->declareVariable("radiance_ray_type")->setUint(0u);
		context->declareVariable("shadow_ray_type")->setUint(1u);
		context->declareVariable("scene_epsilon")->setFloat(1e-4f);

		setupLights();

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
		fps_camera->setSpeed( 5.f );

		createBoringShader();
	}

	void setupLights()
	{
		glm::vec3 color1( 0.3f, 0.3f, 0.3f );
		glm::vec3 color2( 0.4f, 0.4f, 0.4f );

		lights.push_back( light_t( glm::vec3(2.15f,2.00f,0.90f), color1 ) );
		lights.push_back( light_t( glm::vec3(2.35f,2.00f,0.90f), color1 ) );
		lights.push_back( light_t( glm::vec3(2.15f,2.00f,1.10f), color1 ) );
		lights.push_back( light_t( glm::vec3(2.35f,2.00f,1.10f), color1 ) );

		lights.push_back( light_t( glm::vec3(0.25f,1.35f,1.00f), color2 ) );
		lights.push_back( light_t( glm::vec3(4.35f,1.35f,1.00f), color2 ) );
		


		light_buffer_obj = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER, sizeof(light_t) );
		light_buffer_obj->setElementSize( sizeof(light_t) );
		light_buffer_obj->setSize(lights.size());
		void* light_buffer_data = light_buffer_obj->map();
		for (size_t i=0; i<lights.size(); i++) {
			((light_t*)light_buffer_data)[i] = lights[i];
		}
		light_buffer_obj->unmap();
		light_buffer->set(light_buffer_obj);


	}

	void createInstances()
	{
		top_level_group = context->createGroup();
		optix::Variable top_object = context->declareVariable("top_object");
		top_object->set( top_level_group );
		top_level_acceleration = context->createAcceleration("NoAccel", "NoAccel");
		top_level_group->setAcceleration(top_level_acceleration);

		recieve_shadow_group = context->createGroup();
		recieve_shadow_group->setAcceleration( context->createAcceleration("Bvh", "Bvh") );
		optix::Variable top_shadower = context->declareVariable("top_shadower");
		top_shadower->set(recieve_shadow_group);
		addToTopLevel( recieve_shadow_group );

		optix::Material phong_material = createMaterial();
		optix::Material debug_normals_material = createNormalDebugMaterial();
		optix::Material glass_mat = createGlassMaterial();

		optix::Program isect_program = context->createProgramFromPTXFile( optix_dir+"triangle_mesh_small.cu.ptx", "mesh_intersect" );
		optix::Program bbox_program = context->createProgramFromPTXFile( optix_dir+"triangle_mesh_small.cu.ptx", "mesh_bounds" );
		
		//optix::Geometry box = createBoxGeometry();
		//createFloor(box, phong_material);

		std::string model_dir = resource_dir + "models\\";
		File::MeshLoader mesh_loader(model_dir);

		auto icosphereMeshData = mesh_loader.loadMeshDataEasy("icosphere.3ds");
		auto icoGeometryAndMesh = OptixTriMeshLoader::fromMeshData( icosphereMeshData, context, isect_program, bbox_program);
		
		// create ico sphere for each light
		for (size_t i=0; i<lights.size(); ++i){
			auto triMesh = icoGeometryAndMesh.triMesh;
			auto ico_instance = Scene::OptixInstancePtr( new Scene::OptixInstance(triMesh->getVao(), triMesh->getVbo(), triMesh->getIbo(), 
				                                                                  icoGeometryAndMesh.rtGeo, top_level_group, debug_normals_material) );
			ico_instance->setPosition( glm::vec3(lights[i].pos.x,lights[i].pos.y,lights[i].pos.z) );
			scene_instances.push_back(ico_instance);
			scene_light_meshes.push_back(ico_instance);
		}

		ini::Parser config(resource_dir + "ini\\scene.ini");
		auto scene_dir = config.getString("load", "dir", "procedural\\");
		auto scene_file = config.getString("load", "scene", "balls.nff");
		
		asset_manager = File::AssetManagerPtr( new File::AssetManager(resource_dir) );
		File::BARTLoader2 bart_loader( asset_manager, resource_dir+"bart_scenes\\" );

		auto bartNodes = bart_loader.load(scene_dir, scene_file);
		// Key, Value
		std::map<Scene::MeshDataPtr, OptixTriMeshLoader::OptixGeometryAndTriMesh_t> meshdata_optixmesh_map;
		puts("converting scene data to optix and GL objects");
		for(auto it=begin(bartNodes); it!=end(bartNodes); ++it)
		{
			auto &bartNode = *it;
			
			
			auto foundit = meshdata_optixmesh_map.find( bartNode.meshData );
			if ( foundit == meshdata_optixmesh_map.end() ) {
				meshdata_optixmesh_map[bartNode.meshData] = OptixTriMeshLoader::fromMeshData( bartNode.meshData , context, isect_program, bbox_program );
			}

			auto triMesh = meshdata_optixmesh_map[bartNode.meshData].triMesh;
			auto rtGeo = meshdata_optixmesh_map[bartNode.meshData].rtGeo;
			auto optixInstance = Scene::OptixInstancePtr( 
				                 new Scene::OptixInstance( triMesh->getVao(), triMesh->getVbo(), triMesh->getIbo(), 
								                           rtGeo, recieve_shadow_group, debug_normals_material ) );
			optixInstance->setObjectToWorldMatrix( bartNode.xform );
			optixInstance->setMaterial( bartNode.material );

			scene_instances.push_back(optixInstance);
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

	optix::Material createGlassMaterial()
	{
		std::string path_to_ptx = optix_dir + "\\glass.cu.ptx";
		optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
		optix::Program any_hit_program = context->createProgramFromPTXFile( path_to_ptx, "any_hit_shadow" );

		optix::Material glass_matl = context->createMaterial();
		glass_matl->setClosestHitProgram(0, closest_hit_program);
		glass_matl->setAnyHitProgram(1, any_hit_program);

		//glass_matl["importance_cutoff"]->setFloat( 1e-2f );
		glass_matl["cutoff_color"]->setFloat( 0.034f, 0.055f, 0.085f );
		glass_matl["fresnel_exponent"]->setFloat( 3.0f );
		glass_matl["fresnel_minimum"]->setFloat( 0.1f );
		glass_matl["fresnel_maximum"]->setFloat( 1.0f );
		glass_matl["refraction_index"]->setFloat( 1.4f );
		glass_matl["refraction_color"]->setFloat( 1.0f, 1.0f, 1.0f );
		glass_matl["reflection_color"]->setFloat( 1.0f, 1.0f, 1.0f );
		glass_matl["refraction_maxdepth"]->setInt( 10 );
		glass_matl["reflection_maxdepth"]->setInt( 5 );
		glm::vec3 extinction(.83f, .83f, .83f);
		glass_matl["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
		glass_matl["shadow_attenuation"]->setFloat( 0.6f, 0.6f, 0.6f );
		return glass_matl;
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

	void createBoringShader()
	{
		std::string vs = "#version 330 core\n"
		"#define DIFFUSE  0\n"
		"#define POSITION	1\n"
		"#define NORMAL   2\n"
		"#define TEXCOORD	3\n"
	
		"uniform mat4 Object_to_World;\n"
		"uniform mat4 World_to_View;\n"
		"uniform mat4 View_to_Clip;\n"
		"uniform mat3 Normal_to_View;\n"
	
		"layout(location = POSITION) in vec3 Position_os;\n"	//object space
		"layout(location = NORMAL)   in vec3 Normal_os;\n"	//object space
		"layout(location = TEXCOORD) in vec2 TexCoord;\n"

		"out gl_PerVertex\n"
		"{\n"
		" vec4 gl_Position;\n"
		"};\n"

		"out block\n"
		"{"
		" vec4 position_ws;\n"	//world space
		" vec4 position_vs;\n" 	//view space
		" vec3 normal_vs;\n" 	//view space
		" vec2 texcoord;\n"
		"} Vertex;\n"

		"void main( void )\n"
		"{"
		"  Vertex.texcoord      = TexCoord;\n"
		"  Vertex.normal_vs     = normalize(Normal_to_View * Normal_os);\n"		//Object space to View space
	
		"  Vertex.position_ws   = Object_to_World * vec4(Position_os, 1.0);\n"	    //Object space to World space
		"  Vertex.position_vs   = World_to_View * Vertex.position_ws;\n"           //World  space to View  space
		"  gl_Position          = View_to_Clip  * Vertex.position_vs;\n"			//View   space to Clip  space
		// MODEL =      Object_to_World
		// VIEW =       World_to_View
		// PROJECTION = View_to_Clip
		//"  gl_Position          = View_to_Clip * World_to_View * Object_to_World * vec4(Position_os, 1.0);\n"			//View   space to Clip  space
		"}\n";

		std::string fs = "#version 330\n"
			"#define DIFFUSE  0\n"
			"#define POSITION	1\n"
			"#define NORMAL   2\n"
			"#define TEXCOORD	3\n"
			"in block\n"
			"{\n"
			"  vec4 position_ws;\n"	//world space
			"  vec4 position_vs;\n" 	//view space
			"  vec3 normal_vs;\n"    //view space
			"  vec2 texcoord;\n"
			"} Vertex;\n"
			"\n"
			"layout(location = 0, index = 0) out vec4 out_FragColor;\n"
			"void main()\n"
			"{\n"
			"out_FragColor = vec4(Vertex.normal_vs*0.5+0.5, 0.0);\n"
			"}\n";

		boring_shader = std::shared_ptr<Render::Shader>( new Render::Shader(vs, "", fs) );
	}

public:
	Render::PBO *pbo;
private:
	optix::Context                   context;
	optix::Buffer                    out_buffer_obj;
	optix::Variable                  out_buffer_var;
	optix::Variable                  fTime;
	
	optix::Group                     top_level_group;
	optix::Acceleration              top_level_acceleration;
	optix::Group                     recieve_shadow_group;
	
	std::vector<Scene::OptixInstancePtr> scene_instances;
	std::vector<Scene::OptixInstancePtr> scene_light_meshes;

	File::AssetManagerPtr            asset_manager;
	
	std::vector<light_t>             lights;
	optix::Buffer                    light_buffer_obj;
	optix::Variable                  light_buffer;
	Render::ShaderPtr                boring_shader;

	std::string                      optix_dir;
	std::string                      resource_dir;
	int                              width;
	int                              height;
};
