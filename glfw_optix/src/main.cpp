#include <string>
#include <iostream>

#define GLFW_NO_GLU
#define GLFW_INCLUDE_GL3
#include <GL/glfw3.h>
#include <GL3/gl3w.h>

#include <Optix/optix.h>
#include <Optix/optix_gl_interop.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h> 
#include <memory>

#include <Render/Tex2D.h>
#include <Render/Shader.h>
#include <Scene/Quad.h>


// screen, buffer and tex all same size
const static int TEX_WIDTH = 1024;
const static int TEX_HEIGHT =768;

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
			"out_FragColor = vec4(1.0) * texture2D(tex0,t.xy);\n"
			//"out_FragColor = vec4(t.x,t.y,1.0,1.0);\n"
			"}\n";
		screen_quad_shader = std::unique_ptr<Render::Shader>( new Render::Shader(vertex_src, "", fragment_src ) );

	}

	void map( RTbuffer imageBuffer ) 
	{
		RTresult result;
		
		GLsizei width, height;
		RTsize buffer_width, buffer_height;
		RTformat buffer_format;

		result = rtBufferMap(imageBuffer, &imageData);
		if (result != RT_SUCCESS) {
			// Get error from context
			RTcontext context;
			const char* error;
			rtBufferGetContext(imageBuffer, &context);
			rtContextGetErrorString(context, result, &error);
			fprintf(stderr, "Error mapping image buffer: %s\n", error);
			exit(2);
		}
		if (0 == imageData) {
			fprintf(stderr, "data in buffer is null.\n");
			exit(2);
		}

		result = rtBufferGetSize2D(imageBuffer, &buffer_width, &buffer_height);
		if (result != RT_SUCCESS) {
			// Get error from context
			RTcontext context;
			const char* error;
			rtBufferGetContext(imageBuffer, &context);
			rtContextGetErrorString(context, result, &error);
			fprintf(stderr, "Error getting dimensions of buffer: %s\n", error);
			exit(2);
		}
		width  = static_cast<GLsizei>(buffer_width);
		height = static_cast<GLsizei>(buffer_height);

		result = rtBufferGetFormat(imageBuffer, &buffer_format);
		gl_data_type = GL_FALSE;
		gl_format = GL_FALSE;
		switch (buffer_format) {
		case RT_FORMAT_UNSIGNED_BYTE4:
			gl_data_type = GL_UNSIGNED_BYTE;
			gl_format    = GL_BGRA;
			break;

		case RT_FORMAT_FLOAT3:
			gl_data_type = GL_FLOAT;
			gl_format    = GL_RGB;
			break;

		case RT_FORMAT_FLOAT4:
			gl_data_type = GL_FLOAT;
			gl_format    = GL_RGBA; // expected
			break;

		default:
			fprintf(stderr, "Unrecognized buffer data type or format.\n");
			exit(2);
			break;
		}
		Render::T2DTexParams params( GL_RGBA32F, gl_format, gl_data_type, 4, width, height, GL_CLAMP_TO_EDGE, (unsigned char*)imageData ); 
		outputTex.update( params );
	}

	void unmap( RTbuffer imageBuffer )
	{
		// Now unmap the buffer
		RTresult result = rtBufferUnmap(imageBuffer);
		if (result != RT_SUCCESS) {
			// Get error from context
			RTcontext context;
			const char* error;
			rtBufferGetContext(imageBuffer, &context);
			rtContextGetErrorString(context, result, &error);
			fprintf(stderr, "Error unmapping image buffer: %s\n", error);
			exit(2);
		}
	}

	void display(RTbuffer imageBuffer, RTvariable fTime)
	{
		//imageBuffer = buffer;
		RTcontext context;
		rtBufferGetContext(imageBuffer, &context);

		RT_CHECK_ERROR( rtContextValidate( context ) );
			RT_CHECK_ERROR( rtContextCompile( context ) );

		bool running = true;
		while (running)
		{
			float time =(float)glfwGetTime() ;
			/*char titleBuf[512];
			sprintf(titleBuf, "t = %.2f", time );
			glfwSetWindowTitle(wnd, titleBuf );*/
			RT_CHECK_ERROR( rtVariableSet1f( fTime, time ) );

			
			RT_CHECK_ERROR( rtContextLaunch2D( context, 0 /* entry point */, width, height ) );

			map(imageBuffer);
			//glClear(GL_COLOR_BUFFER_BIT);
			// display imagebuffer on a gl quad rendered using a vanilla vs, and a texture samplin' fs.
			
			glActiveTexture(GL_TEXTURE0 + 0);
			outputTex.bind();

			screen_quad_shader->bind();
			int loc_tex0 = glGetUniformLocation( screen_quad_shader->getFS() , "tex0");
			glProgramUniform1i(screen_quad_shader->getFS(), loc_tex0, 0);
			
			screenQuad->render(0);

			glfwSwapBuffers();
			unmap(imageBuffer);

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
	RTcontext context;
    RT_CHECK_ERROR( rtContextCreate( &context ) );
    RT_CHECK_ERROR( rtContextSetRayTypeCount( context, 1 ) );
    RT_CHECK_ERROR( rtContextSetEntryPointCount( context, 1 ) );
	
	RTprogram ray_gen_program;
	std::string path_to_ptx = optix_dir + "\\glfw_optix.cu.ptx";
    RT_CHECK_ERROR( rtProgramCreateFromPTXFile( context, path_to_ptx.c_str(), "sampleTex", &ray_gen_program ) );

	RTvariable launch_dim;
	RT_CHECK_ERROR( rtProgramDeclareVariable( ray_gen_program, "launch_dim", &launch_dim ) );
	RT_CHECK_ERROR( rtVariableSet2ui( launch_dim, width, height ) );

	RTvariable fTime;
	RT_CHECK_ERROR( rtProgramDeclareVariable( ray_gen_program, "fTime", &fTime ) );
	RT_CHECK_ERROR( rtVariableSet1f( fTime, (float)glfwGetTime() ) );

	RT_CHECK_ERROR( rtContextSetRayGenerationProgram( context, 0, ray_gen_program ) );

	RTbuffer  buffer;
	RT_CHECK_ERROR( rtBufferCreate( context, RT_BUFFER_OUTPUT, &buffer ) );
    RT_CHECK_ERROR( rtBufferSetFormat( buffer, RT_FORMAT_FLOAT4 ) );
    RT_CHECK_ERROR( rtBufferSetSize2D( buffer, width, height ) );
	RTvariable out_buffer;
    RT_CHECK_ERROR( rtContextDeclareVariable( context, "out_buffer", &out_buffer ) );
    RT_CHECK_ERROR( rtVariableSetObject( out_buffer, buffer ) );

	//rtCreateBufferFromGLBO(
	//rtBufferCreateFromGLBO(context,RT_BUFFER_OUTPUT,

	myGLContext.display( buffer, fTime );

    /* Clean up */
    RT_CHECK_ERROR( rtBufferDestroy( buffer ) );
    RT_CHECK_ERROR( rtProgramDestroy( ray_gen_program ) );
    RT_CHECK_ERROR( rtContextDestroy( context ) );


	//try {
	//	//kernel->init(argc, argv);
	//} catch( std::exception &e) {
	//	std::cout << e.what() << std::endl;
	//	system("pause");
	//	return -1;
	//}
	
	return 0;
}

