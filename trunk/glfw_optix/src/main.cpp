
//#include <GL3\gl3.h>
//#include <GL3\gl3w.h>

#include <string>
#include <iostream>

#define GLFW_NO_GLU
#include <GL/glfw3.h>
#include <GL/gl.h>

#include <Optix/optix.h>
#include <Optix/optix_gl_interop.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>

// screen, buffer and tex all same size
const static int TEX_WIDTH = 512;
const static int TEX_HEIGHT =384;

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

GLuint getTextureHandle()
{
  GLuint texId = 0;
  glGenTextures( 1, &texId );
  glBindTexture( GL_TEXTURE_2D, texId );
  GLfloat img[TEX_HEIGHT][TEX_WIDTH][4];
  
  //Create a simple checkerboard texture (from OpenGL Programming Guide)
  for( int j = 0; j < TEX_WIDTH; j++ ) {
    for( int i = 0; i < TEX_HEIGHT; i++ ) {
      GLfloat c = ( ( ( i & 0x8 ) == 0 ) ^ ( ( j & 0x8 ) == 0 ) ) * 1.0f;
      img[ i ][ j ][ 0 ] = 1.0f;
      img[ i ][ j ][ 1 ] = c;
      img[ i ][ j ][ 2 ] = c;
      img[ i ][ j ][ 3 ] = 1.0f;

	  float sphx = TEX_WIDTH * .5f;
	  float sphy = TEX_HEIGHT * .5f;
	  float dx = (j-sphx);
	  float dy = (i-sphy);
	  float dist = sqrt(dx*dx + dy*dy);
	  if ( dist < 50.f ) {
		img[ i ][ j ][ 0 ] = 1.0f;
		img[ i ][ j ][ 1 ] = 1.0f;
		img[ i ][ j ][ 2 ] = 1.0f;
		img[ i ][ j ][ 3 ] = 1.0f;
	  }
    }
  }

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGBA, GL_FLOAT, img );

  glBindTexture( GL_TEXTURE_2D, 0 );

  if( glGetError( ) != 0 )
    throw;

  return texId;
}

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
		initState();
	}

	~GLContext()
	{
		glfwTerminate();
		exit(EXIT_SUCCESS);
	}

	void initState()
	{
		// Init state
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.f, (float)width, (float)height, 0.f, -1.f, +1.f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.375, 0.375, 0.0);
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
			//gl_format    = GL_BGRA; // TODO
			break;

		case RT_FORMAT_FLOAT:
			gl_data_type = GL_FLOAT;
			gl_format    = GL_LUMINANCE;
			break;

		case RT_FORMAT_FLOAT3:
			gl_data_type = GL_FLOAT;
			gl_format    = GL_RGB;
			break;

		case RT_FORMAT_FLOAT4:
			gl_data_type = GL_FLOAT;
			gl_format    = GL_RGBA;
			break;

		default:
			fprintf(stderr, "Unrecognized buffer data type or format.\n");
			exit(2);
			break;
		}
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

		bool running = true;
		while (running)
		{

			float time =(float)glfwGetTime() ;
			char titleBuf[512];
			sprintf(titleBuf, "t = %.2f", time );
			glfwSetWindowTitle(wnd, titleBuf );
			RT_CHECK_ERROR( rtVariableSet1f( fTime, time ) );

			RT_CHECK_ERROR( rtContextValidate( context ) );
			RT_CHECK_ERROR( rtContextCompile( context ) );
			RT_CHECK_ERROR( rtContextLaunch2D( context, 0 /* entry point */, width, height ) );

			map(imageBuffer);
			//glClear(GL_COLOR_BUFFER_BIT);
			glDrawPixels(width, height, gl_format, gl_data_type, imageData);
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
	
  // Create a texture sampler with the OpenGL texture as input.
	
    RTtexturesampler textureSampler;
    RT_CHECK_ERROR( rtTextureSamplerCreateFromGLImage(context, getTextureHandle(), RT_TARGET_GL_TEXTURE_2D, &textureSampler));
	
	// RT_WRAP_CLAMP_TO_EDGE RT_WRAP_REPEAT

	RT_CHECK_ERROR( rtTextureSamplerSetWrapMode(textureSampler, 0, RT_WRAP_CLAMP_TO_EDGE) );
	RT_CHECK_ERROR( rtTextureSamplerSetWrapMode(textureSampler, 1, RT_WRAP_CLAMP_TO_EDGE) );
	RT_CHECK_ERROR( rtTextureSamplerSetWrapMode(textureSampler, 2, RT_WRAP_CLAMP_TO_EDGE) ) ;
	//  RT_TEXTURE_INDEX_NORMALIZED_COORDINATES,
    //RT_TEXTURE_INDEX_ARRAY_INDEX
	RT_CHECK_ERROR( rtTextureSamplerSetIndexingMode(textureSampler, RT_TEXTURE_INDEX_NORMALIZED_COORDINATES ) );
	RT_CHECK_ERROR( rtTextureSamplerSetReadMode(textureSampler, RT_TEXTURE_READ_NORMALIZED_FLOAT ) );
	RT_CHECK_ERROR( rtTextureSamplerSetMaxAnisotropy(textureSampler, 1.0f ) );
	RT_CHECK_ERROR( rtTextureSamplerSetFilteringModes(textureSampler, RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE ) );
	
	RTvariable tex;
	//RT_CHECK_ERROR( rtProgramDeclareVariable( ray_gen_program, "tex", &tex ) );
	RT_CHECK_ERROR( rtContextDeclareVariable( context, "tex", &tex ) );
	RT_CHECK_ERROR( rtVariableSetObject( tex, textureSampler ) );
	
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

