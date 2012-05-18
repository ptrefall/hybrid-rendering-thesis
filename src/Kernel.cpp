#include "Kernel.h"
#include "Parser\INIParser.h"

#include "Render\DeferredRender.h"
#include "Render\GBuffer.h"
#include "Render\Shader.h"
#include "Raytracer\OptixRender.h"
#include "File\ShaderLoader.h"
#include "File\TextureLoader.h"
#include "File\MaterialLoader.h"
#include "Scene\SceneManager.h"
#include "Scene\Cube.h"
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

	long shader_loader_count = shader_loader.use_count();
	shader_loader.reset();
	long tex_loader_count = tex_loader.use_count();
	tex_loader.reset();
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
	shader_loader = std::make_shared<File::ShaderLoader>(resource_dir+"shaders\\");
	tex_loader = std::make_shared<File::TextureLoader>(resource_dir+"textures\\");
	mat_loader = std::make_shared<File::MaterialLoader>(resource_dir+"materials\\");


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

	//Rasterize
	glEnable(GL_DEPTH_TEST);
	g_buffer->begin();
	scene->render(g_buffer->getShader());
	g_buffer->end();
	glDisable(GL_DEPTH_TEST);
	renderer->render();

    //Raytrace
    raytracer->render();
	//tex_loader->save(nullptr, resource_dir+"screens\\MRT.png");
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
	camera->updateProjection(width, height, 40.0f, 1.0f, 1000.0f);
    //camera->init(width, height, 60.0f, 1.0f, 1000.0f);
	//camera->setTarget(glm::vec3(10,-8,20));

	//auto cube_tex = tex_loader->load("cube.jpg", GL_REPEAT);

	//auto array_tex = tex_loader->load_array("array.png", 16, 16, 2, 2, GL_REPEAT);
	auto array_tex = tex_loader->load("cube.jpg");
	auto array2_tex = tex_loader->load("array.png");
	Render::UniformPtr tex_sampler = std::make_shared<Render::Uniform>(g_buffer->getShader()->getFS(), "diffuse_tex");
	Render::SamplerPtr array_sampler;// = std::make_shared<Render::Sampler>();

	//tex_loader->save(array2_tex, resource_dir+"screens\\array.png");

	auto basic_cube_mat = renderer->addMaterial(mat_loader->load("basic_cube.mat"));
	auto red_cube_mat = renderer->addMaterial(mat_loader->load("red_cube.mat"));
	auto blue_cube_mat = renderer->addMaterial(mat_loader->load("blue_cube.mat"));

	Scene::CubePtr cube;
	cube = std::make_shared<Scene::Cube>(1.0f);
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

	for ( int i=0; i<3; i++ ) {
		for ( int j=0; j<3; j++ ) {
			float x = 10*(-.5f + i/3.f);
			float z = 10*(-.5f + j/3.f);

			Scene::CubePtr cube2 = std::make_shared<Scene::Cube>(.5f);
			{
				cube2->setMVP(	g_buffer->getMVP());
				cube2->setMV(	g_buffer->getMV());
				cube2->setN_WRI(	g_buffer->getN_WRI());
				cube2->setTexture(array_tex, tex_sampler, array_sampler);
				cube2->setMaterial(blue_cube_mat);
				scene->add(cube2);
				cube2->setPosition( glm::vec3(5 + x,-3,-20 + z) );
			}
		}
	}
}
