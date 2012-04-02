#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL/freeglut.h>

#include "Render\DeferredRender.h"
#include "Render\GBuffer.h"
#include "File\ShaderLoader.h"
#include "Scene\SceneManager.h"
#include "Scene\Cube.h"

#include <string>

void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
int init(int argc, char** argv);
void loadScene();

//Globals available only to this file
namespace 
{
	Render::GBufferPtr g_buffer;
	Render::DeferredRenderPtr renderer;
	File::ShaderLoaderPtr shader_loader;
	Scene::SceneManagerPtr scene;

	Scene::CubePtr cube;

	unsigned int width, height;
}

int main(int argc, char** argv)
{
	//////////////////////////////////////////
	// GLUT INITIALIZING
	//////////////////////////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion (4, 2);
	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	width = 800;
	height = 600;

	glutInitWindowSize (width, height); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow (argv[0]);

	//////////////////////////////////////////
	// GL3W INITIALIZING
	//////////////////////////////////////////
	GLenum gl3wInitErr = gl3wInit();
	if(gl3wInitErr)
		throw std::runtime_error("Failed to initialize OpenGL!");
	if(gl3wIsSupported(3,3) == false)
		throw std::runtime_error("OpenGL 3.3 is not supported!");

	//////////////////////////////////////////
	// GAME INITIALIZING
	//////////////////////////////////////////
	if(init(argc, argv))
		return -1; 

	//////////////////////////////////////////
	// GLUT FUNCTOR INITIALIZING
	//////////////////////////////////////////
	glutIdleFunc(display);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	//////////////////////////////////////////
	// HEARTBEAT
	//////////////////////////////////////////
	glutMainLoop();

	return 0;
}

void display()
{
	float color_buffer_clear[4];
	memset(color_buffer_clear, 0.0f, sizeof(float)*4);
	float depth_buffer_clear = 0.0f;
	glClearBufferfv(GL_COLOR, 0, color_buffer_clear);
	glClearBufferfv(GL_DEPTH, 0, &depth_buffer_clear);

	g_buffer->begin();
	scene->render();
	g_buffer->end();

	renderer->render();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_buffer->reshape(w,h);
	renderer->reshape(w,h);
}

void keyboard(unsigned char key, int x, int y)
{
}

int init(int argc, char** argv)
{
	//////////////////////////////////////////////
	// SET UP SAFE RESOURCE BASE DIRECTORY LOOKUP
	//////////////////////////////////////////////
	std::string base_dir = argv[0];
	auto pos = base_dir.find_last_of("/");
	if(pos == base_dir.npos)
		pos = base_dir.find_last_of("\\");
	if(pos == base_dir.npos)
		return -1;
	base_dir = base_dir.substr(0, pos);
	
	pos = base_dir.find_last_of("/");
	if(pos == base_dir.npos)
		pos = base_dir.find_last_of("\\");
	if(pos == base_dir.npos)
		return -1;
	base_dir = base_dir.substr(0, pos+1);

	base_dir = base_dir + "resources\\";

	//////////////////////////////////////////
	// FILE SYSTEM INITIALIZING
	//////////////////////////////////////////
	shader_loader = std::make_shared<File::ShaderLoader>(base_dir+"shaders\\");


	//////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
	g_buffer = std::make_shared<Render::GBuffer>(shader_loader, width, height);
	renderer = std::make_shared<Render::DeferredRender>(g_buffer, shader_loader, width, height);

	//////////////////////////////////////////
	// SCENE INITIALIZING
	//////////////////////////////////////////
	scene = std::make_shared<Scene::SceneManager>();
	loadScene();
	return 0;
}

void loadScene()
{
	cube = std::make_shared<Scene::Cube>(2.0f);
	{
		cube->setMVP(	g_buffer->getMVP());
		cube->setMV(	g_buffer->getMV());
		cube->setN_WRI(	g_buffer->getN_WRI());
		scene->add(cube);
	}
}