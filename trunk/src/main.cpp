#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL/freeglut.h>

#include "Render\DeferredRender.h"
#include "Render\GBuffer.h"
#include "Raytracer\OptixRender.h"
#include "File\ShaderLoader.h"
#include "File\TextureLoader.h"
#include "File\MaterialLoader.h"
#include "Scene\SceneManager.h"
#include "Scene\Cube.h"
#include "Scene\Camera.h"

#include "Render\Shader.h"

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
    Raytracer::OptixRenderRenderPtr raytracer;
	File::ShaderLoaderPtr shader_loader;
	File::TextureLoaderPtr tex_loader;
	File::MaterialLoaderPtr mat_loader;
	Scene::SceneManagerPtr scene;
  
    Scene::CameraPtr camera;
	

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

	width = 1280;
	height = 800;

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
	memset(color_buffer_clear, 1.0f, sizeof(float)*4);
	float depth_buffer_clear = 0.0f;
  
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
    //raytracer->render();


	glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_buffer->reshape(w,h);
	renderer->reshape(w,h);
}

void keyboard(unsigned char key, int x, int y)
{
	if ( key=='R' || key=='r' ) {
		system("cls");
		renderer->reloadShaders();
	}
}

int init(int argc, char** argv)
{
	//////////////////////////////////////////////
	// SET UP SAFE RESOURCE BASE DIRECTORY LOOKUP
	//////////////////////////////////////////////
	std::string base_dir(argv[0]);
	base_dir = base_dir.substr(0, base_dir.find_last_of("\\"));
	base_dir = base_dir.substr(0, base_dir.find_last_of("\\"));
	base_dir += "\\resources\\";

	//////////////////////////////////////////
	// FILE SYSTEM INITIALIZING
	//////////////////////////////////////////
	shader_loader = std::make_shared<File::ShaderLoader>(base_dir+"shaders\\");
	tex_loader = std::make_shared<File::TextureLoader>(base_dir+"textures\\");
	mat_loader = std::make_shared<File::MaterialLoader>(base_dir+"materials\\");


	//////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
	g_buffer = std::make_shared<Render::GBuffer>(shader_loader, width, height);
	renderer = std::make_shared<Render::DeferredRender>(g_buffer, shader_loader, width, height);

    //////////////////////////////////////////
	// DEFERRED RENDERER INITIALIZING
	//////////////////////////////////////////
    raytracer = std::make_shared<Raytracer::OptixRender>(g_buffer, width, height, base_dir + "optix\\");

	//////////////////////////////////////////
	// SCENE INITIALIZING
	//////////////////////////////////////////
	scene = std::make_shared<Scene::SceneManager>();
	loadScene();
	return 0;
}

void loadScene()
{
    camera = Scene::Camera::getSingleton();
    camera->init(width, height, M_PI/3.0f, 1.0f, 1000.0f);

	auto cube_tex = tex_loader->load("cube.jpg");
	auto tex_sampler = std::make_shared<Render::Uniform>(g_buffer->getShader()->getFS(), "diffuse_tex");

	auto basic_cube_mat = renderer->addMaterial(mat_loader->load("basic_cube.mat"));
	auto red_cube_mat = renderer->addMaterial(mat_loader->load("red_cube.mat"));

	Scene::CubePtr cube;
	cube = std::make_shared<Scene::Cube>(1.0f);
	{
		cube->setMVP(	g_buffer->getMVP());
		cube->setMV(	g_buffer->getMV());
		cube->setN_WRI(	g_buffer->getN_WRI());
		cube->setTexture(cube_tex, tex_sampler);
		cube->setMaterial(basic_cube_mat);
		scene->add(cube);
		cube->setPosition( Eigen::Vector3f(10,-8,20) );
	}

	Scene::CubePtr cube2 = std::make_shared<Scene::Cube>(.5f);
	{
		cube2->setMVP(	g_buffer->getMVP());
		cube2->setMV(	g_buffer->getMV());
		cube2->setN_WRI(	g_buffer->getN_WRI());
		cube2->setTexture(cube_tex, tex_sampler);
		cube2->setMaterial(red_cube_mat);
		scene->add(cube2);
		cube2->setPosition( Eigen::Vector3f(10,-4,20) );
	}
}
