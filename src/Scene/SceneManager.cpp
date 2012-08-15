#include "SceneManager.h"
#include "Light.h"
#include "proto_camera.h"

#include "Spacejet.h"
#include "Cube.h"
#include "CubeSphere.h"
#include "BARTMesh.h"
#include "OptixInstance.h"
#include "../Raytracer/OptixTriMeshLoader.h"
#include "../Raytracer/OptixRender.h"
#include "../Render/Passes/Raytrace/Raytrace_Pass.h"

#include "../Render/Passes/Final/Final_Pass.h"
#include "../Render/Passes/GBuffer/GBuffer_Pass.h"
#include "../Render/Passes/Bloom/Bloom_Pass.h"
#include "../Render/Passes/Raytrace/Raytrace_Pass.h"
#include "../Render/DebugOutput.h"
#include "../Render/GPUTimer.h"

#include "../Parser/INIParser.h"

#include <algorithm>
#include <map>
#include <ctime>

using namespace Scene;

SceneManager::SceneManager(const File::ShaderLoaderPtr &shader_loader, unsigned int width, unsigned int height, const std::string &resource_dir)
	: shader_loader(shader_loader), width(width), height(height), resource_dir(resource_dir)
{
	debug_output = std::make_shared<Render::DebugOutput>();
	//gpu_timer = std::make_shared<Render::GPUTimer>();

	//////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
	g_buffer_pass = std::make_shared<Render::GBuffer_Pass>(shader_loader, width, height);
	raytrace_pass = std::make_shared<Render::Raytrace_Pass>(g_buffer_pass, width, height, resource_dir);
	//bloom_pass = std::make_shared<Render::Bloom_Pass>(final_pass, shader_loader, width, height);
	final_pass = std::make_shared<Render::Final_Pass>(g_buffer_pass, raytrace_pass, shader_loader, width, height);
}

SceneManager::~SceneManager()
{
	long final_pass_count = final_pass.use_count();
	final_pass.reset();

	/*long bloom_pass_count = bloom_pass.use_count();
	bloom_pass.reset();*/

	long raytrace_pass_count = raytrace_pass.use_count();
	raytrace_pass.reset();

	long g_buffer_pass_count = g_buffer_pass.use_count();
	g_buffer_pass.reset();
}

void SceneManager::render()
{
	//static int frame_count = 0;

	//static int g_buffer_pass_accum_time = 0;
	//int g_buffer_pass_dt_time = 0;
	//gpu_timer->begin();
	g_buffer_pass->begin();
	{
		glEnable(GL_DEPTH_TEST);
		for(auto it = begin(scene); it!=end(scene); ++it )
			(*it)->render(g_buffer_pass->getShader());
		glDisable(GL_DEPTH_TEST);
	} g_buffer_pass->end();
	/*gpu_timer->end();
	while(!gpu_timer->isResultAvailable()) {}
	g_buffer_pass_dt_time = gpu_timer->getTimeElapsed();
	g_buffer_pass_accum_time += g_buffer_pass_dt_time;*/


	/*static int raytrace_pass_accum_time = 0;
	int raytrace_pass_dt_time = 0;
	gpu_timer->begin();*/
	raytrace_pass->begin();
	{
	//	//raytrace_pass->update(g_buffer_pass->getRenderTextures(), scene, lights);
		raytrace_pass->render();
	} raytrace_pass->end();
	/*while(!gpu_timer->isResultAvailable()) {}
	raytrace_pass_dt_time = gpu_timer->getTimeElapsed();
	raytrace_pass_accum_time += raytrace_pass_dt_time;*/

	/*light_pass->begin();
	{
		light_pass->update(lights, g_buffer_pass->getRenderTextures(), raytrace_pass->getRenderTextures());
		light_pass->render();
	} light_pass->end();*/

	/*bloom_pass->begin();
	{
		bloom_pass->update(light_pass->getFinalColorTexture());
		bloom_pass->render_extraction_step();
		bloom_pass->render_blur_steps();
		bloom_pass->render_final_step();
	} bloom_pass->end();*/

	/*static int final_pass_accum_time = 0;
	int final_pass_dt_time = 0;
	gpu_timer->begin();*/
	final_pass->begin();
	{
		//final_pass->update(light_pass->getFinalColorTexture(), bloom_pass->getFilterTexture());
		bindLights(final_pass->getShader());
		final_pass->render();
	} final_pass->end();
	/*while(!gpu_timer->isResultAvailable()) {}
	final_pass_dt_time = gpu_timer->getTimeElapsed();
	final_pass_accum_time += final_pass_dt_time;*/

	//std::cout << frame_count << " " << g_buffer_pass_dt_time << " " << raytrace_pass_dt_time << " " << final_pass_dt_time << " " << g_buffer_pass_accum_time << " " << raytrace_pass_accum_time << " " << final_pass_accum_time << std::endl;
	//frame_count++;
}

void SceneManager::bindLights(const Render::ShaderPtr &active_program)
{
	for(auto it = begin(lights); it!=end(lights); ++it )
		(*it)->bind(active_program);
}

void SceneManager::reshape(int width, int height)
{
	g_buffer_pass->reshape(width,height);
	raytrace_pass->reshape(width,height);
	final_pass->reshape(width,height);
}

void SceneManager::add(const SceneNodePtr &node)
{
	scene.push_back(node);
}

void SceneManager::addList(const std::vector<SceneNodePtr> &nodeList)
{
	scene.insert( scene.end(), nodeList.begin(), nodeList.end() );
}

void SceneManager::add(const LightPtr &light)
{
	lights.push_back(light);
}

void SceneManager::initScene(	const File::AssetManagerPtr &asset_manager, 
								const File::MaterialLoaderPtr &mat_loader, 
								const File::MeshLoaderPtr &mesh_loader,
								const File::BARTLoader2Ptr &bart_loader)
{
	auto camera = Scene::FirstPersonCamera::getSingleton();
	camera->updateProjection(width, height, 40.0f, 0.1f, 1000.0f);
    //camera->init(width, height, 60.0f, 1.0f, 1000.0f);
	//camera->setTarget(glm::vec3(10,-8,20));

	Scene::LightPtr light = std::make_shared<Scene::Light>(0);
	light->setPosition(glm::vec3(0,0,0));

	auto spacejet_tex = asset_manager->getTex2DRelativePath("FEROX_DI.tga", false);
	auto spacejet_normal_tex = asset_manager->getTex2DRelativePath("FEROX_BU.tga", false);

	auto array_tex = asset_manager->getTex2DRelativePath("cube.jpg", false);
	auto array2_tex = asset_manager->getTex2DRelativePath("array.png", false);
	
	Render::UniformPtr tex_sampler = std::make_shared<Render::Uniform>(g_buffer_pass->getShader()->getFS(), "diffuse_tex");
	Render::UniformPtr norm_sampler = std::make_shared<Render::Uniform>(g_buffer_pass->getShader()->getFS(), "normal_tex");
	
	Render::SamplerPtr array_sampler;// = std::make_shared<Render::Sampler>();

    //final_pass->setRayTexture(raytracer->getRenderTexture(), tex_sampler);

	auto basic_cube_mat = final_pass->addMaterial(mat_loader->load("basic_cube.mat"));
	auto red_cube_mat = final_pass->addMaterial(mat_loader->load("red_cube.mat"));
	auto blue_cube_mat = final_pass->addMaterial(mat_loader->load("blue_cube.mat"));

	auto spacejet = mesh_loader->load<Scene::Spacejet>("Ferox.3DS");
	{
		spacejet->init(shader_loader);
		spacejet->setTexture(0,spacejet_tex, "diffuse_tex");
		spacejet->setTexture(1,spacejet_normal_tex, "normal_tex");
		spacejet->setMaterial(red_cube_mat);
		spacejet->setPosition(glm::vec3(0, 0, 20));
		spacejet->setScale(glm::vec3(0.01f,0.01f,0.01f));
		add(spacejet);
	}

	Scene::CubePtr cube = std::make_shared<Scene::Cube>(1.0f);
	{
		cube->setObjectToWorldUniform(	g_buffer_pass->getObjectToWorldUniform());
		cube->setWorldToViewUniform(	g_buffer_pass->getWorldToViewUniform());
		cube->setViewToClipUniform(		g_buffer_pass->getViewToClipUniform());
		cube->setNormalToViewUniform(	g_buffer_pass->getNormalToViewUniform());
		cube->setTexture(0, array_tex, tex_sampler, array_sampler);
		cube->setMaterial(basic_cube_mat);
		add(cube);
		cube->setPosition( glm::vec3(5,5,-20) );
	}

	Scene::CubePtr cube2 = std::make_shared<Scene::Cube>(.5f);
	{
		cube2->setObjectToWorldUniform(	g_buffer_pass->getObjectToWorldUniform());
		cube2->setWorldToViewUniform(	g_buffer_pass->getWorldToViewUniform());
		cube2->setViewToClipUniform(	g_buffer_pass->getViewToClipUniform());
		cube2->setNormalToViewUniform(	g_buffer_pass->getNormalToViewUniform());
		cube2->setTexture(0,array2_tex, tex_sampler, array_sampler);
		cube2->setMaterial(red_cube_mat);
		add(cube2);
		cube2->setPosition( glm::vec3(5,3,-20) );
	}

    const int sideBySide = 25;
    const float fSideBySide = float(sideBySide);
	for ( int i=0; i<sideBySide; i++ ) {
		for ( int j=0; j<sideBySide; j++ ) {
            const float u = -.5f + i/fSideBySide;
            const float v = -.5f + j/fSideBySide;
            float freq = 2.f * 6.28f;
            float distOrigin = sqrt(u*u + v*v);
            const float x = fSideBySide*u;
            const float y = 1.5f * cos(distOrigin * freq) - 10.f;
			const float z = fSideBySide*v;
            
			Scene::CubePtr cube3 = std::make_shared<Scene::Cube>(0.5f);
			{
				cube3->setObjectToWorldUniform(	g_buffer_pass->getObjectToWorldUniform());
				cube3->setWorldToViewUniform(	g_buffer_pass->getWorldToViewUniform());
				cube3->setViewToClipUniform(	g_buffer_pass->getViewToClipUniform());
				cube3->setNormalToViewUniform(	g_buffer_pass->getNormalToViewUniform());
				cube3->setTexture(0, array_tex, tex_sampler, array_sampler);
				cube3->setMaterial(blue_cube_mat);
				add(cube3);
				cube3->setPosition( glm::vec3(x,y,z) );
			}
		}
	}

	Scene::CubeSpherePtr sphere = std::make_shared<Scene::CubeSphere>();
	{
		sphere->setObjectToWorldUniform(	g_buffer_pass->getObjectToWorldUniform());
		sphere->setWorldToViewUniform(	g_buffer_pass->getWorldToViewUniform());
		sphere->setViewToClipUniform(	g_buffer_pass->getViewToClipUniform());
		sphere->setNormalToViewUniform(	g_buffer_pass->getNormalToViewUniform());
		sphere->setTexture(0,array_tex, tex_sampler, array_sampler);
		sphere->setMaterial(blue_cube_mat);
		add(sphere);
		sphere->setPosition( glm::vec3(1,-1,6) );
	}

	ini::Parser config(resource_dir + "ini\\scene.ini");
	auto scene_dir = config.getString("load", "dir", "procedural\\");
	auto scene_file = config.getString("load", "scene", "balls.nff");

	// Key, Value
	std::map<Scene::MeshDataPtr, OptixTriMeshLoader::OptixGeometryAndTriMesh_t> meshdata_optixmesh_map;
	puts("converting scene data to optix and GL objects");

	// We need a context to create rtGeometry, and programs
	optix::Context context = raytrace_pass->getRaytracer()->getContext();
	const std::string optix_dir = resource_dir + "\\Optix";
	optix::Program isect_program = context->createProgramFromPTXFile( optix_dir+"\\triangle_mesh_small.cu.ptx", "mesh_intersect" );
	optix::Program bbox_program = context->createProgramFromPTXFile( optix_dir+"\\triangle_mesh_small.cu.ptx", "mesh_bounds" );

	// Create debug-normals Material Program, will later be replaced with anyhit for shadow
	std::string path_to_ptx = optix_dir + "\\mat_normal.cu.ptx";
	optix::Program closest_hit_program = context->createProgramFromPTXFile( path_to_ptx, "closest_hit_radiance" );
	//optix::Program any_hit_program = context->createProgramFromPTXFile( path_to_ptx, "any_hit_shadow" );
	optix::Material debug_normals_material = context->createMaterial();
	debug_normals_material->setClosestHitProgram(0 /*radiance*/, closest_hit_program); // debug normals only uses closest hit.
	//debug_normals_material->setAnyHitProgram(0 /*shadow*/, any_hit_program); // debug normals only uses closest hit.
	
	// Create the group all trace-able geometry is to be a child of
	optix::Group top_level_group = context->createGroup();
	optix::Variable top_object = context->declareVariable("top_object");
	top_object->set( top_level_group );
	optix::Acceleration top_level_acceleration = context->createAcceleration("Bvh", "Bvh"); // can only use Bvh with current GLBO format
	top_level_group->setAcceleration(top_level_acceleration);

	auto bartNodes = bart_loader->load(scene_dir, scene_file);
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
								                        rtGeo, top_level_group, debug_normals_material ) );
		optixInstance->setObjectToWorldMatrix( bartNode.xform );
		optixInstance->setMaterial( bartNode.material );
		
		this->add(optixInstance);

		// also add trimesh to scene
		// todo: this doesnt handle instances. only uniques...
#if 1 
		triMesh->setMaterial( bartNode.material );
		triMesh->setObjectToWorldMatrix( bartNode.xform );
		triMesh->setObjectToWorldUniform(	g_buffer_pass->getObjectToWorldUniform());
		triMesh->setWorldToViewUniform(	g_buffer_pass->getWorldToViewUniform());
		triMesh->setViewToClipUniform(		g_buffer_pass->getViewToClipUniform());
		triMesh->setNormalToViewUniform(	g_buffer_pass->getNormalToViewUniform());
		if ( bartNode.textureFilename != "" ) {
			Render::UniformPtr tex_sampler = std::make_shared<Render::Uniform>(this->getGBufferPass()->getShader()->getFS(), "diffuse_tex");
			auto tex2d = asset_manager->getTex2DAbsolutePath( bartNode.textureFilename, true );
			Render::SamplerPtr dummy_sampler; // TODO
			triMesh->setTexture(0, tex2d, tex_sampler, dummy_sampler ); 
		}
		this->add(triMesh);
#endif
	}

	puts("validating context...");
	try{
		raytrace_pass->getRaytracer()->getContext()->validate();
	}catch(const optix::Exception &e){
		std::cout << e.getErrorString();
		system("pause");
		exit(1);
	}

	puts("compiling context...");
	clock_t timeStart = clock();
	raytrace_pass->getRaytracer()->compileContext();
	clock_t timeEnd = clock()-timeStart;
	std::cout << "optix compile took " << timeEnd/(double)CLOCKS_PER_SEC << " seconds " << std::endl;

}
