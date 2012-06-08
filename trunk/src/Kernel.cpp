#include "Kernel.h"
#include "Parser\INIParser.h"

#include "Render\Shader.h"
#include "File\AssetManager.h"
#include "File\ShaderLoader.h"
#include "File\MaterialLoader.h"
#include "File\BARTLoader2.h"
#include "File\MeshLoader.h"
#include "Scene\SceneManager.h"
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
	wgl3wSwapIntervalEXT(0);

	//////////////////////////////////////////
	// FILE SYSTEM INITIALIZING
	//////////////////////////////////////////
	asset_manager = std::make_shared<File::AssetManager>(resource_dir);
	shader_loader = std::make_shared<File::ShaderLoader>(resource_dir+"shaders\\");
	mat_loader = std::make_shared<File::MaterialLoader>(resource_dir+"materials\\");
	bart_loader = std::make_shared<File::BARTLoader2>(asset_manager, resource_dir+"bart_scenes\\");
	mesh_loader = std::make_shared<File::MeshLoader>(resource_dir+"models\\");

	//////////////////////////////////////////
	// SCENE INITIALIZING
	//////////////////////////////////////////
	scene = std::make_shared<Scene::SceneManager>(shader_loader, width, height, resource_dir);
	scene->initScene(asset_manager, mat_loader, mesh_loader, bart_loader);
	
	camera = Scene::FirstPersonCamera::getSingleton();
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
	camera->update(keystatus['a'], keystatus['d'], keystatus['s'], keystatus['w'], glm::vec2(mouse.coords), mouse.leftPressed, dt);
}

void Kernel::render()
{
	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	scene->render();
}

void Kernel::reshape(int w, int h)
{
	glViewportIndexedf(0,0.0f, 0.0f, (float)w, (float)h);

	scene->reshape(w,h);
}

void Kernel::inputKeyDown(unsigned char key, int x, int y)
{
	//ESCAPE KEY
	if(key == 27)
		exit();

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
