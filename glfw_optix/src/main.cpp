#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

#include <string>
//#include <iostream>
#include <memory>

#define GLFW_NO_GLU
#define GLFW_INCLUDE_GL3
#include <GL/glfw3.h>
#include <GL3/gl3w.h>

#include <Optix/optixu/optixpp_namespace.h>
#include <Render/Tex2D.h>
#include <Render/Shader.h>
#include <Render/PBO.h>
#include <Scene/Quad.h>


// screen, buffer and tex all same size
const static int TEX_WIDTH = 1920;
const static int TEX_HEIGHT =1080;

class OptixScene
{
public:
	OptixScene(std::string optix_dir, int width, int height)
	{
		/* Create our objects and set state */
		context = optix::Context::create();
		context->setRayTypeCount(1);
		context->setEntryPointCount(1);

		std::string path_to_ptx = optix_dir + "\\glfw_optix.cu.ptx";
		optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "sampleTex" );

		unsigned int screenDims[] = {width,height};
		ray_gen_program->declareVariable("rtLaunchDim")->set2uiv(screenDims);

		fTime = ray_gen_program->declareVariable("fTime");
		float initTime = (float)glfwGetTime();
		fTime->set1fv( &initTime );

		context->setRayGenerationProgram(0, ray_gen_program);

		// Create shared GL/CUDA PBO
		int element_size = 4 * sizeof(char);
		pbo = new Render::PBO(element_size * width * height, GL_STREAM_DRAW, true);

		buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, pbo->getHandle() );
		buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
		buffer->setSize(width,height);

		optix::Variable out_buffer = context->declareVariable("out_buffer");
		out_buffer->set(buffer);
	}

	optix::Variable getFTime()
	{
		return fTime;
	}

	optix::Buffer getOutBuffer()
	{
		return buffer;
	}

	optix::Program getRayGen()
	{
		return context->getRayGenerationProgram(0);
	}

	~OptixScene()
	{
		/* Clean up */
		try {
			buffer->unregisterGLBuffer();
			buffer->destroy();
			getRayGen()->destroy();
			context->destroy();
		} catch( std::exception &e) {
			//std::cout << e.what() << std::endl;
			printf("%s\n", e.what() );
			system("pause");
		}
	}
public:
	Render::PBO *pbo;
private:
	optix::Context context;
	optix::Buffer buffer;
	optix::Variable fTime;
};

class GLContext
{
public:
	GLContext(int width, int height) 
		: width(width)
		, height(height)
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

		glfwSetWindowCloseCallback(GLContext::closeWindow);
		glfwSetWindowUserPointer(wnd, this);

		//////////////////////////////////////////
		// GL3W INITIALIZING
		//////////////////////////////////////////
		GLenum gl3wInitErr = gl3wInit();
		if(gl3wInitErr)
			throw std::runtime_error("Failed to initialize OpenGL!");
		if(gl3wIsSupported(3, 3) == false)
			throw std::runtime_error("Opengl 3.3 is not supported!");
		wgl3wSwapIntervalEXT(0);

		initState();
	}

	~GLContext()
	{
		glfwTerminate();
		exit(EXIT_SUCCESS);
	}

	void initState()
	{
		screenQuad = std::unique_ptr<Scene::Quad>( new Scene::Quad(0,0) );

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
			"out_FragColor = vec4(1.0) * texture(tex0,t.xy);\n"
			//"out_FragColor = vec4(t.x,t.y,1.0,1.0);\n"
			"}\n";
		screen_quad_shader = std::unique_ptr<Render::Shader>( new Render::Shader(vertex_src, "", fragment_src ) );

		Render::T2DTexParams params( GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, 4, width, height, GL_CLAMP_TO_EDGE, (unsigned char*) nullptr ); 
		outputTex.init( params );
	}

	void pbo2Tex(optix::Buffer buffer)
	{
		outputTex.bind();

		int buffer_width = width;
		int buffer_height = height;

		optix::Context context = buffer->getContext();

		Render::PBO *pbo = myScene->pbo;
		pbo->bind(true);

		RTsize elementSize = buffer->getElementSize();
		RTformat buffer_format = buffer->getFormat();
		
		if      ((elementSize % 8) == 0) pbo->align(8);
		else if ((elementSize % 4) == 0) pbo->align(4);
		else if ((elementSize % 2) == 0) pbo->align(2);
		else                             pbo->align(1);

		if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT3) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
		} else {
		  throw "Unknown buffer format";
		}

		pbo->unbind();
	}

	void destroy()
	{
		delete myScene;
	}

	static int closeWindow(GLFWwindow wnd)
	{
		GLContext *ptr = (GLContext*)glfwGetWindowUserPointer(wnd);
		ptr->destroy();
		return 1;
	}

	void display(OptixScene *myScene)
	{
		this->myScene = myScene;
		
		optix::Buffer imageBuffer = myScene->getOutBuffer();
		optix::Variable fTime = myScene->getFTime();
		optix::Context context = imageBuffer->getContext();
		context->validate();
		context->compile();

		bool running = true;
		while (running)
		{
			float time =(float)glfwGetTime() ;
			/*char titleBuf[512];
			sprintf(titleBuf, "t = %.2f", time );
			glfwSetWindowTitle(wnd, titleBuf );*/
			fTime->set1fv( &time );

			context->launch(0 /*entry point*/, width, height );

			glActiveTexture(GL_TEXTURE0 + 0);
			pbo2Tex(imageBuffer);
			//glClear(GL_COLOR_BUFFER_BIT);
			// display imagebuffer on a gl quad rendered using a vanilla vs, and a texture samplin' fs.
				
			outputTex.bind();
			screen_quad_shader->bind();
			int loc_tex0 = glGetUniformLocation( screen_quad_shader->getFS() , "tex0");
			glProgramUniform1i(screen_quad_shader->getFS(), loc_tex0, 0);
			
			screenQuad->render(0);

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

		Render::Tex2D outputTex;
		std::unique_ptr<Scene::Quad> screenQuad;
		std::unique_ptr<Render::Shader> screen_quad_shader;

		OptixScene *myScene;		
};

int main(int argc, char* argv[])
{
	std::string resource_dir = argv[0];
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir = resource_dir.substr(0, resource_dir.find_last_of("\\"));
	resource_dir += "\\resources\\";

	std::string optix_dir = resource_dir + "\\glfw_optix\\";

	unsigned int width = TEX_WIDTH;
	unsigned int height = TEX_HEIGHT;

	GLContext myGLContext(width,height);
	OptixScene *myScene = new OptixScene(optix_dir, width, height);
	myGLContext.display( myScene );

	return 0;
}

