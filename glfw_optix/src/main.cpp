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
#include <Scene/Quad.h> 


// screen, buffer and tex all same size
const static int TEX_WIDTH = 1920;
const static int TEX_HEIGHT =1080;

GLuint vbo = 0;

void sutilReportError(const char* message)
{
  fprintf( stderr, "OptiX Error: %s\n", message );
#if defined(_WIN32) && defined(RELEASE)
  {
    char s[2048];
    sprintf( s, "OptiX Error: %s", message );
    MessageBox( 0, s, "OptiX Error", MB_OK|MB_ICONWARNING|MB_SYSTEMMODAL );
  }
#endif
}

void sutilHandleErrorNoExit(RTcontext context, RTresult code, const char* file, int line)
{
  const char* message;
  char s[2048];
  rtContextGetErrorString(context, code, &message);
  sprintf(s, "%s\n(%s:%d)", message, file, line);
  sutilReportError( s );
}
void sutilHandleError(RTcontext context, RTresult code, const char* file, int line)
{
  sutilHandleErrorNoExit( context, code, file, line );
  exit(1);
}
#define RT_CHECK_ERROR( func )                                     \
  do {                                                             \
    RTresult code = func;                                          \
    if( code != RT_SUCCESS )                                       \
      sutilHandleError( context, code, __FILE__, __LINE__ );       \
  } while(0)

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

		GLuint vboId = buffer->getGLBOId();
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId );

		RTsize elementSize = buffer->getElementSize();

		RTformat buffer_format = buffer->getFormat();
		
		if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 
		else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT4) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
		} else if(buffer_format == RT_FORMAT_FLOAT3) {
		  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
		} else {
		  throw "Unknown buffer format";
		}

		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
	}

	void display(optix::Buffer imageBuffer, optix::Variable fTime)
	{
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
			if (!glfwIsWindow(wnd) )
				running = false;	
		}
	}

	private:
		int width, height;
		GLFWwindow wnd;
		GLvoid* imageData;
		GLenum gl_data_type;
		GLenum gl_format;
		Render::Tex2D outputTex;
		std::unique_ptr<Scene::Quad> screenQuad;
		std::unique_ptr<Render::Shader> screen_quad_shader;
		
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
	
    /* Create our objects and set state */
	optix::Context context = optix::Context::create();
	context->setRayTypeCount(1);
	context->setEntryPointCount(1);

	std::string path_to_ptx = optix_dir + "\\glfw_optix.cu.ptx";
	optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "sampleTex" );
	
	unsigned int screenDims[] = {width,height};
	ray_gen_program->declareVariable("rtLaunchDim")->set2uiv(screenDims);

	optix::Variable fTime = ray_gen_program->declareVariable("fTime");
	float initTime = (float)glfwGetTime();
	fTime->set1fv( &initTime );

	context->setRayGenerationProgram(0, ray_gen_program);
	
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int element_size = 4 * sizeof(char);
	glBufferData(GL_ARRAY_BUFFER, element_size * width * height, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	optix::Buffer buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
	buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	buffer->setSize(width,height);

	optix::Variable out_buffer = context->declareVariable("out_buffer");
	out_buffer->set(buffer);
	
	myGLContext.display( buffer, fTime );

    /* Clean up */
	try {
		buffer->unregisterGLBuffer();
		buffer->destroy();
		ray_gen_program->destroy();
		context->destroy();
	} catch( std::exception &e) {
		//std::cout << e.what() << std::endl;
		printf("%s\n", e.what() );
		system("pause");
		return -1;
	}
	
	return 0;
}

