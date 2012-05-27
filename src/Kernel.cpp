#include "Kernel.h"
#include "Parser\INIParser.h"

#include "Render\DeferredRender.h"
#include "Render\GBuffer.h"
#include "Render\Shader.h"
#include "Raytracer\OptixRender.h"
#include "File\AssetManager.h"
#include "File\ShaderLoader.h"
#include "File\MaterialLoader.h"
#include "File\BARTLoader2.h"
#include "Scene\SceneManager.h"
#include "Scene\Cube.h"
#include "Scene\Light.h"
#include "Scene\proto_camera.h"

#include <GL3\gl3w.h>

#include <sstream>

#include "config.h"

KernelPtr Kernel::singleton;

KernelPtr Kernel::getSingleton()
{
	if(singleton == nullptr)
		singleton = std::make_shared<Kernel>();
	return singleton;
}

void Kernel::Shutdown()
{
	if(singleton)
	{
		long count = singleton.use_count();
		singleton.reset();
	}
}

Kernel::Kernel()
{
	memset(keystatus, 0, MAX_KEYS);
	mouse.leftPressed = false;
	mouse.rightPressed = false;
}

Kernel::~Kernel()
{
	long renderer_count = renderer.use_count();
	renderer.reset();

	long raytracer_count = raytracer.use_count();
	raytracer.reset();

	long g_buffer_count = g_buffer.use_count();
	g_buffer.reset();

	long scene_count = scene.use_count();
	scene.reset();

	
	long asset_manager_count = asset_manager.use_count();
	asset_manager.reset();
	long shader_loader_count = shader_loader.use_count();
	shader_loader.reset();
	long mat_loader_count = mat_loader.use_count();
	mat_loader.reset();

	Scene::FirstPersonCamera::Shutdown();
	long camera_count = camera.use_count();
	camera.reset();
}

std::string Kernel::getGameModeString() const
{
	std::stringstream ss;
	ss << width << "x" << height << ":" << depth << "@" << refresh_rate;
	return ss.str();
}

std::string Kernel::getOpenGLVersionString() const
{
	std::stringstream ss;
	ss << opengl_major_version << "." << opengl_minor_version;
	return ss.str();
}

void Kernel::config(const std::string &resource_dir)
{
	this->resource_dir = resource_dir;

	ini::Parser parser(resource_dir + "ini\\engine.ini");
	width = parser.getInt("Dimensions", "width", ENGINE_DEFAULT_WINDOW_WIDTH);
	height = parser.getInt("Dimensions", "height", ENGINE_DEFAULT_WINDOW_HEIGHT);
	depth = parser.getInt("Dimensions", "depth", ENGINE_DEFAULT_WINDOW_DEPTH);
	refresh_rate = parser.getInt("Dimensions", "refresh_rate", ENGINE_DEFAULT_WINDOW_REFRESH_RATE);
	logic_update_rate = parser.getInt("Dimensions", "logic_update_rate", ENGINE_DEFAULT_LOGIC_UPDATE_RATE);
	fullscreen = parser.getInt("Modes", "fullscreen", ENGINE_DEFAULT_FULLSCREEN);
	game_mode = parser.getInt("Modes", "game", ENGINE_DEFAULT_GAME_MODE);
	opengl_major_version = parser.getInt("OpenGL", "major_version", ENGINE_OPENGL_VERSION_MAJOR);
	opengl_minor_version = parser.getInt("OpenGL", "minor_version", ENGINE_OPENGL_VERSION_MINOR);
}

void Kernel::init(int argc, char** argv)
{
	//////////////////////////////////////////
	// GL3W INITIALIZING
	//////////////////////////////////////////
	GLenum gl3wInitErr = gl3wInit();
	if(gl3wInitErr)
		throw std::runtime_error("Failed to initialize OpenGL!");
	if(gl3wIsSupported(opengl_major_version, opengl_minor_version) == false)
		throw std::runtime_error("Opengl " + getOpenGLVersionString() + " is not supported!");

	//////////////////////////////////////////
	// FILE SYSTEM INITIALIZING
	//////////////////////////////////////////
	asset_manager = std::make_shared<File::AssetManager>(resource_dir);
	shader_loader = std::make_shared<File::ShaderLoader>(resource_dir+"shaders\\");
	mat_loader = std::make_shared<File::MaterialLoader>(resource_dir+"materials\\");
	bart_loader = std::make_shared<File::BARTLoader2>(asset_manager, resource_dir+"bart_scenes\\");

	//////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
	g_buffer = std::make_shared<Render::GBuffer>(shader_loader, width, height);
	renderer = std::make_shared<Render::DeferredRender>(g_buffer, shader_loader, width, height);

    //////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
    raytracer = std::make_shared<Raytracer::OptixRender>(g_buffer, width, height, resource_dir + "optix\\");

	//////////////////////////////////////////
	// SCENE INITIALIZING
	//////////////////////////////////////////
	scene = std::make_shared<Scene::SceneManager>();
	initScene();
}

void Kernel::run(int start_time, std::function<void()> main_loop_body)
{
	this->start_time = start_time;
	prev_time = start_time;

	running = true;
	while(running)
		main_loop_body();

	Kernel::Shutdown();
}

void Kernel::update(float dt)
{
	camera->update(keystatus['a'], keystatus['d'], keystatus['s'], keystatus['w'], mouse.coords.x, mouse.coords.y, mouse.leftPressed, dt);
}

void Kernel::render()
{
	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Raytrace
    raytracer->render();

	//Rasterize
	glEnable(GL_DEPTH_TEST);
	g_buffer->begin();
		scene->render(g_buffer->getShader());
	g_buffer->end();
	glDisable(GL_DEPTH_TEST);

	renderer->begin();
		scene->bindLights(renderer->getShader());
		renderer->render();
	renderer->end();
}

void Kernel::reshape(int w, int h)
{
	glViewportIndexedf(0,0.0f, 0.0f, (float)w, (float)h);

	g_buffer->reshape(w,h);
	renderer->reshape(w,h);
}

void Kernel::inputKeyDown(unsigned char key, int x, int y)
{
	//ESCAPE KEY
	if(key == 27)
		exit();
	else if(key == 'r')
		renderer->reloadShaders();

	keystatus[key] = true;
	//mouse.coords = glm::ivec2(x,y);
}

void Kernel::inputKeyUp(unsigned char key, int x, int y)
{
	keystatus[key] = false;
	//mouse.coords = glm::ivec2(x,y);
}

void Kernel::input(int key, int x, int y)
{
	// Special keys ALT CTRL etc.
	//mouse.coords = glm::ivec2(x,y);
}

void Kernel::motion(int x, int y)
{
	mouse.coords = glm::ivec2(x,y);
}

void Kernel::mousePressed(int button, int state, int x, int y)
{
	const int LEFT_BUTTON = 0;
	const int MIDDLE_BUTTON = 1;
	const int RIGHT_BUTTON = 2;
	const int PRESSED = 0;
	const int RELEASED = 1;
	if(button==LEFT_BUTTON)
	{
		mouse.leftPressed = (state==PRESSED);
	} 
	else if(button==RIGHT_BUTTON)
	{
		mouse.rightPressed = (state==PRESSED);
	}

	//mouse.coords = glm::ivec2(x,y);
}

void Kernel::initScene()
{
	camera = Scene::FirstPersonCamera::getSingleton();
	camera->updateProjection(width, height, 40.0f, 0.1f, 1000.0f);
    //camera->init(width, height, 60.0f, 1.0f, 1000.0f);
	//camera->setTarget(glm::vec3(10,-8,20));

	Scene::LightPtr light = std::make_shared<Scene::Light>(0);
	light->setPosition(glm::vec3(0,0,0));

	auto array_tex = asset_manager->getTex2DRelativePath("cube.jpg", false);
	auto array2_tex = asset_manager->getTex2DRelativePath("array.png", false);
	Render::UniformPtr tex_sampler = std::make_shared<Render::Uniform>(g_buffer->getShader()->getFS(), "diffuse_tex");
	Render::SamplerPtr array_sampler;// = std::make_shared<Render::Sampler>();

    renderer->setRayTexture(raytracer->getRenderTexture(), tex_sampler);

	auto basic_cube_mat = renderer->addMaterial(mat_loader->load("basic_cube.mat"));
	auto red_cube_mat = renderer->addMaterial(mat_loader->load("red_cube.mat"));
	auto blue_cube_mat = renderer->addMaterial(mat_loader->load("blue_cube.mat"));

	Scene::CubePtr cube = std::make_shared<Scene::Cube>(1.0f);
	{
		cube->setMVP(	g_buffer->getMVP());
		cube->setMV(	g_buffer->getMV());
		cube->setN_WRI(	g_buffer->getN_WRI());
		cube->setTexture(raytracer->getRenderTexture(), tex_sampler, array_sampler);
		cube->setMaterial(basic_cube_mat);
		scene->add(cube);
		cube->setPosition( glm::vec3(5,5,-20) );
	}

	Scene::CubePtr cube2 = std::make_shared<Scene::Cube>(.5f);
	{
		cube2->setMVP(	g_buffer->getMVP());
		cube2->setMV(	g_buffer->getMV());
		cube2->setN_WRI(	g_buffer->getN_WRI());
		cube2->setTexture(array2_tex, tex_sampler, array_sampler);
		cube2->setMaterial(red_cube_mat);
		scene->add(cube2);
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
				cube3->setMVP(	g_buffer->getMVP());
				cube3->setMV(	g_buffer->getMV());
				cube3->setN_WRI(	g_buffer->getN_WRI());
				cube3->setTexture(array_tex, tex_sampler, array_sampler);
				cube3->setMaterial(blue_cube_mat);
				scene->add(cube3);
				cube3->setPosition( glm::vec3(x,y,z) );
			}
		}
	}

	ini::Parser config(resource_dir + "ini\\scene.ini");
	auto scene_dir = config.getString("load", "dir", "procedural\\");
	auto scene_file = config.getString("load", "scene", "balls.nff");

	std::vector<Scene::SceneNodePtr> nodes = bart_loader->load(scene_dir, scene_file);
	for(auto it=begin(nodes); it!=end(nodes); ++it)
	{
		Scene::SceneNodePtr &node = *it;
		node->setMVP(	g_buffer->getMVP());
		node->setMV(	g_buffer->getMV());
		node->setN_WRI(	g_buffer->getN_WRI());
		//node->setTexture(array_tex, tex_sampler, array_sampler);
	}
	scene->addList( nodes );
}
