
#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL\freeglut.h>
#include <string>

#include "config.h"
#include "Parser\INIParser.h"

void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void special(int key, int x, int y);
void init(int argc, char** argv);

int main(int argc, char** argv)
{
	//////////////////////////////////////////////
	// SET UP SAFE RESOURCE BASE DIRECTORY LOOKUP
	//////////////////////////////////////////////
	std::string resource_dir = argv[0];
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir += "\\resources\\";

	//////////////////////////////////////////////
	// PARSE ENGINE SETTINGS
	//////////////////////////////////////////////
	ini::Parser parser(resource_dir + "ini\\engine.ini");
	int width = parser.getInt("Dimensions", "width", ENGINE_DEFAULT_WINDOW_WIDTH);
	int height = parser.getInt("Dimensions", "height", ENGINE_DEFAULT_WINDOW_HEIGHT);
	int depth = parser.getInt("Dimensions", "depth", ENGINE_DEFAULT_WINDOW_DEPTH);
	int refresh_rate = parser.getInt("Dimensions", "refresh_rate", ENGINE_DEFAULT_WINDOW_REFRESH_RATE);
	int fullscreen = parser.getInt("Modes", "fullscreen", ENGINE_DEFAULT_FULLSCREEN);
	int game_mode = parser.getInt("Modes", "game", ENGINE_DEFAULT_GAME_MODE);

	//////////////////////////////////////////
	// GLUT INITIALIZING
	//////////////////////////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion (ENGINE_OPENGL_VERSION_MAJOR, ENGINE_OPENGL_VERSION_MINOR);
	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	if(game_mode)
	{
		std::stringstream ss;
		ss << width << "x" << height << ":" << depth << "@" << refresh_rate;
		glutGameModeString(ss.str().c_str());
		if(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
			glutEnterGameMode();
		else
		{
			system("pause");
			return -1;
		}
	}
	else 	
	{
		glutInitWindowSize (width, height); 
		glutInitWindowPosition (100, 100);
		glutCreateWindow(argv[0]);
		if(fullscreen)
			glutFullScreen();
	}

	//////////////////////////////////////////
	// GL3W INITIALIZING
	//////////////////////////////////////////
	GLenum gl3wInitErr = gl3wInit();
	if(gl3wInitErr)
		throw std::runtime_error("Failed to initialize OpenGL!");
	if(gl3wIsSupported(ENGINE_OPENGL_VERSION_MAJOR,ENGINE_OPENGL_VERSION_MINOR) == false)
	{
		std::stringstream ss;
		ss << "Opengl " << ENGINE_OPENGL_VERSION_MAJOR << "." << ENGINE_OPENGL_VERSION_MINOR << " is not supported!";
		throw std::runtime_error(ss.str());
	}

	//////////////////////////////////////////
	// GLUT FUNCTOR INITIALIZING
	//////////////////////////////////////////
	glutIdleFunc(display);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

	//////////////////////////////////////////
	// CUSTOM INITIALIZATION
	//////////////////////////////////////////
	init(argc,argv);

	//////////////////////////////////////////
	// HEARTBEAT
	//////////////////////////////////////////
	glutMainLoop();
}

void display()
{
	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//CUSTOM RENDER CODE GOES HERE

	glutSwapBuffers();
}

void reshape(int w, int h)
{
}

void keyboard(unsigned char key, int x, int y)
{
	//ESCAPE KEY
	if(key == 27)
		glutLeaveMainLoop();
}

void special(int key, int x, int y)
{
}

void init(int argc, char** argv)
{
	//CUSTOM INITIALIZATION CODE GOES HERE
}
