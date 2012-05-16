#include "Kernel.h"
#include "config.h"
#include "Parser\INIParser.h"

#include <GL3\gl3w.h>

#include <sstream>

KernelPtr Kernel::singleton;

KernelPtr Kernel::getSingleton()
{
	if(singleton == nullptr)
		singleton = std::make_shared<Kernel>();
	return singleton;
}

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

std::string Kernel::getGameModeString() const
{
	std::stringstream ss;
	ss << width << "x" << height << ":" << depth << "@" << refresh_rate;
	return ss.str();
}

int Kernel::getOpenGLVersionMajor() const
{
	return ENGINE_OPENGL_VERSION_MAJOR;
}

int Kernel::getOpenGLVersionMinor() const
{
	return ENGINE_OPENGL_VERSION_MINOR;
}

std::string Kernel::getOpenGLVersionString() const
{
	std::stringstream ss;
	ss << ENGINE_OPENGL_VERSION_MAJOR << "." << ENGINE_OPENGL_VERSION_MINOR;
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
	fullscreen = parser.getInt("Modes", "fullscreen", ENGINE_DEFAULT_FULLSCREEN);
	game_mode = parser.getInt("Modes", "game", ENGINE_DEFAULT_GAME_MODE);
}

void Kernel::init(int argc, char** argv)
{

}

void Kernel::render()
{
	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Kernel::reshape(int w, int h)
{
}

void Kernel::input(unsigned char key, int x, int y)
{
}
void Kernel::input(int key, int x, int y)
{
}

