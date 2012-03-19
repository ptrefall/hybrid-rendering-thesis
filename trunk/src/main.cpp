#include <GL3\gl3.h>
#include <GL3\gl3w.h>
#include <GL/freeglut.h>

#include "Render\DeferredRender.h"
#include "Render\GBuffer.h"

void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
int init(int argc, char** argv);

Render::GBufferPtr g_buffer;
Render::DeferredRenderPtr renderer;

unsigned int width, height;

int main(int argc, char** argv)
{
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

	GLenum gl3wInitErr = gl3wInit();
	if(gl3wInitErr)
		throw std::runtime_error("Failed to initialize OpenGL!");
	if(gl3wIsSupported(3,3) == false)
		throw std::runtime_error("OpenGL 3.3 is not supported!");

	if(init(argc, argv))
		return -1; 

	glutIdleFunc(display);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
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
	//scene->render();
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
	g_buffer = std::make_shared<Render::GBuffer>(width, height);
	renderer = std::make_shared<Render::DeferredRender>(g_buffer, width, height);
	return 0;
}
