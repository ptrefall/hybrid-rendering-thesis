#include "triangle_scene.h"

#include <string>
#include <iostream>
#include <memory>



#include <Render/Tex2D.h>
#include <Render/Shader.h>
#include <Render/PBO.h>
#include <Scene/Quad.h>
#include <Scene/proto_camera.h>

#define GLFW_NO_GLU
#define GLFW_INCLUDE_GL3
#include <GL/glfw3.h>
#include <GL3/gl3w.h>
#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

const static int SCREEN_WIDTH  = 1024;
const static int SCREEN_HEIGHT = 768;

class ScreenBufferRender
{
public:
	ScreenBufferRender(OptixScene *myScene)
		: myScene(myScene)
		, old_time( 0.0 )
	{
		screenQuad = std::unique_ptr<Scene::Quad>( new Scene::Quad() );

		std::string vertex_src = \
			"#version 330\n"
			"layout(location = 1) in vec2 position;\n"
			"layout(location = 0) in vec2 texcoord;\n"
			"out vec2 t;\n"
			"void main ()\n"
			"{\n"
			"gl_Position = vec4( position.x, position.y, 0.0, 1.0);\n"
			"vec2 madd = vec2(0.5,0.5);\n"
			"vec2 pos_norm = position; // vertices go from 0,0 to 1,1\n"
			"t = (pos_norm * madd) + madd; // Scale to 0-1 range\n"
			"}\n";
		
		std::string fragment_src = \
			"#version 330\n"
			"in vec2 t;\n"
			"layout(location = 0, index = 0) out vec4 out_FragColor;\n"
			"uniform sampler2D tex0;\n"
			"void main()\n"
			"{\n"
			"out_FragColor = texture(tex0,t.xy);\n"
			//"out_FragColor = vec4(t.x,t.y,1.0,1.0);\n"
			"}\n";
		screen_quad_shader = std::unique_ptr<Render::Shader>( new Render::Shader(vertex_src, "", fragment_src ) );

		optix::Buffer imageBuffer = myScene->getOutBuffer();
		RTsize width, height;
		imageBuffer->getSize(width, height);

		Render::T2DTexParams params( GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, 4, width, height, GL_CLAMP_TO_EDGE, (unsigned char*) nullptr ); 
		outputTex.init( params );

		printf("compiling context...\n");
		double timeStart = glfwGetTime();
		myScene->compileScene();
		double timeSpent = glfwGetTime() - timeStart;
		printf("finished compile, took %.1f seconds\n", timeSpent);
	}

	void pbo2Tex()
	{
		outputTex.bind();

		optix::Buffer imageBuffer = myScene->getOutBuffer();
		RTsize width, height;
		imageBuffer->getSize(width, height);

		myScene->pbo->bind(true);

		RTsize elementSize = imageBuffer->getElementSize();
		RTformat buffer_format = imageBuffer->getFormat();
		
		if      ((elementSize % 8) == 0) myScene->pbo->align(8);
		else if ((elementSize % 4) == 0) myScene->pbo->align(4);
		else if ((elementSize % 2) == 0) myScene->pbo->align(2);
		else                             myScene->pbo->align(1);

		if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT3) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
		} else {
		  throw "Unknown buffer format";
		}

		myScene->pbo->unbind();
	}

	void render(GLFWwindow wnd)
	{
		double time = glfwGetTime();
		double delta_time = time - old_time;
		old_time = time;

		optix::Variable fTime = myScene->getFTime();
		fTime->setFloat( (float)time );

		int mouse_x, mouse_y;
		glfwGetMousePos(wnd, &mouse_x, &mouse_y );
		myScene->moveCameraVertical( glfwGetKey(wnd, 'Q')==1, glfwGetKey(wnd, 'E')==1, delta_time );
		myScene->updateCamera( glfwGetKey(wnd, 'A')==1, glfwGetKey(wnd, 'D')==1, glfwGetKey(wnd, 'S')==1, glfwGetKey(wnd, 'W')==1,
			                    glm::vec2((float)mouse_x, (float)mouse_y), glfwGetMouseButton(wnd,0)==1, delta_time );
		myScene->animate();
			
		RTsize width, height;
		myScene->getOutBuffer()->getSize(width, height);
		optix::Context context = myScene->getContext();
		context->launch(0 /*entry point*/, width, height );

		glActiveTexture(GL_TEXTURE0 + 0);
		pbo2Tex();
		// display imagebuffer on a gl quad rendered using a vanilla vs, and a texture samplin' fs.
				
		outputTex.bind();
		screen_quad_shader->bind();
		int loc_tex0 = glGetUniformLocation( screen_quad_shader->getFS() , "tex0");
		glProgramUniform1i(screen_quad_shader->getFS(), loc_tex0, 0);

		screenQuad->render();

		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		myScene->renderRaster();
	}
	
	void resize(int w, int h)
	{
		myScene->resize(w,h);
	}

	void destroy()
	{
		delete myScene;
	}

private:
	Render::Tex2D outputTex;
	std::unique_ptr<Scene::Quad> screenQuad;
	std::unique_ptr<Render::Shader> screen_quad_shader;
	double old_time;
	OptixScene *myScene;
};

class GLContext
{
public:
	GLContext(int width, int height) 
		: width(width)
		, height(height)
		, myRender(nullptr)
	{
		if (!glfwInit())
		{
			fprintf(stderr, "Failed to initialize GLFW: %s\n",
					glfwErrorString(glfwGetError()));
			exit(EXIT_FAILURE);
		}

		wnd = glfwOpenWindow(width, height, GLFW_WINDOWED, "Ray", NULL);

        if (!wnd)
        {
            fprintf(stderr, "Failed to open GLFW window: %s\n",
                    glfwErrorString(glfwGetError()));
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

		glfwMakeContextCurrent(wnd);
		glfwSetWindowUserPointer(wnd, this);
		glfwSetWindowCloseCallback(GLContext::closeWindow);
		glfwSetWindowSizeCallback(GLContext::resizeWindow);
		
		//////////////////////////////////////////
		// GL3W INITIALIZING
		//////////////////////////////////////////
		GLenum gl3wInitErr = gl3wInit();
		if(gl3wInitErr)
			throw std::runtime_error("Failed to initialize OpenGL!");
		if(gl3wIsSupported(3, 3) == false)
			throw std::runtime_error("Opengl 3.3 is not supported!");
		wgl3wSwapIntervalEXT(0);

	}

	~GLContext()
	{
		glfwTerminate();
	}

	void destroy()
	{
		myRender->destroy();
	}

	void resize(int w, int h)
	{
		this->width = w;
		this->height = h;

		if ( myRender ) myRender->resize(w,h);
	}

	static int closeWindow(GLFWwindow wnd)
	{
		GLContext *ptr = (GLContext*)glfwGetWindowUserPointer(wnd);
		ptr->destroy();
		return 1;
	}

	static void resizeWindow(GLFWwindow wnd, int w, int h)
	{
		GLContext *ptr = (GLContext*)glfwGetWindowUserPointer(wnd);
		ptr->resize(w,h);
	}

	void display( ScreenBufferRender *myRender )
	{
		this->myRender = myRender;

		bool running = true;
		while (running)
		{
			myRender->render(wnd);
			glfwSwapBuffers();

			glfwPollEvents();
			if (!glfwIsWindow(wnd) ) {
				running = false;
			}
		}
	}

	private:
		int width, height;
		GLFWwindow wnd;

		ScreenBufferRender *myRender;		
};

int main(int argc, char* argv[])
{
	std::string resource_dir = argv[0];
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir += "\\resources\\";

	unsigned int width = SCREEN_WIDTH;
	unsigned int height = SCREEN_HEIGHT;

	GLContext myGLContext(width,height);
	
	try {
		OptixScene *myScene = new OptixScene(resource_dir, width, height);
		ScreenBufferRender myRender(myScene);
		myGLContext.display( &myRender );
	} catch( std::exception &e) {
		std::cout << e.what() << std::endl;
		system("pause");
	}

	glfwTerminate();

	return 0;
}

