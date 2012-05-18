
#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL\freeglut.h>

#include "Kernel.h"

#include <string>
#include <iostream>

void update(int value);
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void special(int key, int x, int y);
void motion(int x, int y);
void mousePressed(int button, int state, int posX, int posY);
void close();

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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion(kernel->getOpenGLVersionMajor(), kernel->getOpenGLVersionMinor());
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
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
		glutInitWindowSize(kernel->getWidth(), kernel->getHeight()); 
		glutInitWindowPosition(100, 100);
		glutCreateWindow(argv[0]);
		if(kernel->getFullscreen())
			glutFullScreen();
	}

	//////////////////////////////////////////
	// KERNEL INITIALIZATION
	//////////////////////////////////////////'
	try {
		kernel->init(argc, argv);
	} catch( std::exception &e) {
		std::cout << e.what() << std::endl;
		system("pause");
		return -1;
	}

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
	glutTimerFunc(kernel->getLogicUpdateRate(), update, 0);
	glutCloseFunc(close);

	//////////////////////////////////////////
	// HEARTBEAT
	//////////////////////////////////////////
	kernel->run(
		glutGet(GLUT_ELAPSED_TIME), 
		[&](){
			glutMainLoopEvent();
			if(kernel->isRunning())
				glutPostRedisplay();
		});

	//////////////////////////////////////////
	// CLEAN UP
	//////////////////////////////////////////
	kernel.reset();

	//////////////////////////////////////////
	// DESTROY GL CONTEXT AND GLUT WINDOW
	//////////////////////////////////////////
	glutExit();
}

void update(int value)
{
	auto kernel = Kernel::getSingleton();
	glutTimerFunc(kernel->getLogicUpdateRate(), update, 0);

	kernel->update((float)kernel->getTimeSincePrevFrame(glutGet(GLUT_ELAPSED_TIME))/1000.0f);
}

void display()
{
	Kernel::getSingleton()->render();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	Kernel::getSingleton()->reshape(w,h);
}

void keyboard(unsigned char key, int x, int y)
{
	Kernel::getSingleton()->inputKeyDown(key, x,y);
}

void keyboardUp(unsigned char key, int x, int y)
{
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

void close()
{
	Kernel::getSingleton()->exit();
}
