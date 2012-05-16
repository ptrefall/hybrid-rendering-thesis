
#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL\freeglut.h>

#include "config.h"
#include "Kernel.h"

#include <string>


void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void special(int key, int x, int y);
void motion(int x, int y);
void mousePressed(int button, int state, int posX, int posY);

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
	// CONFIGURE KERNEL
	//////////////////////////////////////////////
	auto kernel = Kernel::getSingleton();
	kernel->config(resource_dir);

	//////////////////////////////////////////
	// GLUT INITIALIZING
	//////////////////////////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion (ENGINE_OPENGL_VERSION_MAJOR, ENGINE_OPENGL_VERSION_MINOR);
	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	if(kernel->getGameMode())
	{
		glutGameModeString(kernel->getGameModeString().c_str());
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
		glutInitWindowSize (kernel->getWidth(), kernel->getHeight()); 
		glutInitWindowPosition (100, 100);
		glutCreateWindow(argv[0]);
		if(kernel->getFullscreen())
			glutFullScreen();
	}

	//////////////////////////////////////////
	// GL3W INITIALIZING
	//////////////////////////////////////////
	GLenum gl3wInitErr = gl3wInit();
	if(gl3wInitErr)
		throw std::runtime_error("Failed to initialize OpenGL!");
	if(gl3wIsSupported(ENGINE_OPENGL_VERSION_MAJOR,ENGINE_OPENGL_VERSION_MINOR) == false)
		throw std::runtime_error("Opengl " + kernel->getOpenGLVersionString() + " is not supported!");

	//////////////////////////////////////////
	// GLUT FUNCTOR INITIALIZING
	//////////////////////////////////////////
	glutIdleFunc(display);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutMouseFunc(mousePressed);

	//////////////////////////////////////////
	// KERNEL INITIALIZATION
	//////////////////////////////////////////
	kernel->init(argc, argv);

	//////////////////////////////////////////
	// HEARTBEAT
	//////////////////////////////////////////
	glutMainLoop();
}

void display()
{
	

	//CUSTOM RENDER CODE GOES HERE
	Kernel::getSingleton()->render();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	Kernel::getSingleton()->reshape(w,h);
}

void keyboard(unsigned char key, int x, int y)
{
	//ESCAPE KEY
	if(key == 27)
		glutLeaveMainLoop();

	Kernel::getSingleton()->inputKeyDown(key, x,y);
}

void keyboardUp(unsigned char key, int x, int y)
{
	//ESCAPE KEY
	if(key == 27)
		glutLeaveMainLoop();

	Kernel::getSingleton()->inputKeyUp(key, x,y);
}

void special(int key, int x, int y)
{
	Kernel::getSingleton()->input(key, x,y);
}

void motion(int x, int y)
{
	Kernel::getSingleton()->motion(x,y);
}

void mousePressed(int button, int state, int x, int y)
{
	Kernel::getSingleton()->mousePressed(button, state, x, y);
}


