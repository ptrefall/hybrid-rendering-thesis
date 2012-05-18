#include "OptixRender.h"
#include "../Scene/proto_camera.h"
//#include "../Kernel.h"
//#include "../File/TextureLoader.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

using namespace Raytracer;
using namespace optix;

OptixRender::OptixRender(const Render::GBufferPtr &g_buffer, unsigned int w, unsigned int h, const std::string& baseDir)
	: g_buffer(g_buffer), w(w), h(h), baseDir(baseDir)
{
	//tex = std::make_shared<Render::Tex2D>();

    context  = createContext();
    sphere   = createGeometry( context );
    material = createMaterial( context );
    createInstance( context, sphere, material );
    
    // Run
	try{
    context->validate();
    context->compile();
    //context->launch( 0, w, h);
	}catch(const optix::Exception &e){
	}
}

void OptixRender::_displayFrame( Buffer buffer )
{
  // Draw the resulting image
  RTsize buffer_width_rts, buffer_height_rts;
  buffer->getSize( buffer_width_rts, buffer_height_rts );
  int buffer_width  = static_cast<int>(buffer_width_rts);
  int buffer_height = static_cast<int>(buffer_height_rts);
  RTformat buffer_format = buffer->getFormat();

 GLvoid* imageData = buffer->map();
    assert( imageData );

    GLenum gl_data_type = GL_FALSE;
    GLenum gl_format = GL_FALSE;

    switch (buffer_format) {
          case RT_FORMAT_UNSIGNED_BYTE4:
            gl_data_type = GL_UNSIGNED_BYTE;
            gl_format    = GL_BGRA;
            break;

          case RT_FORMAT_FLOAT:
            gl_data_type = GL_FLOAT;
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
    
	//Render::T2DTexParams params((unsigned int)gl_format, (unsigned int)gl_format, (unsigned int)gl_data_type, 4, (unsigned int)buffer_width, (unsigned int)buffer_height, (unsigned int)GL_CLAMP_TO_EDGE, (unsigned char*)imageData);
	//tex->update(params);

	//Kernel::getSingleton()->getTextureLoader()->save(tex, Kernel::getSingleton()->getResourceDir()+"screens\\raytraced.png");
	
    /*static GLuint idk = 0;
    if ( !idk ) {
      glGenTextures(1, &idk );
    }
    glBindTexture( GL_TEXTURE_2D, idk );
    // Change these to GL_LINEAR for super- or sub-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, buffer_width, buffer_height, gl_format, gl_data_type, imageData );*/
    //glDrawPixels( static_cast<GLsizei>( buffer_width),
      //static_cast<GLsizei>( buffer_height ),
      //gl_format, gl_data_type, imageData);
    
    buffer->unmap();
}

void OptixRender::render()
{
  //unsigned int pbo = context["output_buffer"]->getBuffer()->getGLBOId(); // not a pbo!
	auto camera = Scene::FirstPersonCamera::getSingleton();
	auto &view = camera->getViewMatrix();
	auto &pos = camera->getPos();
 
	context["eye"]->setFloat( pos.x, pos.y, pos.z );
	context["U"]->setFloat( view[0][0], view[1][0], view[2][0] );
	context["V"]->setFloat( view[0][1], view[1][1], view[2][1] );
	context["W"]->setFloat( view[0][2], view[1][2], view[2][2] );

	try {
		context->launch(0, w,h);
	} catch (optix::Exception& e) {
		std::cout << e.getErrorString();
		return;
	}
	_displayFrame( context["output_buffer"]->getBuffer() );
	//glClearColor(1,0,0,0);
}

void OptixRender::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}

Context OptixRender::createContext() 
{
  // Set up context
  Context context = Context::create();
  context->setRayTypeCount( 1 );
  context->setEntryPointCount( 1 );

  context["radiance_ray_type"]->setUint( 0u );
  context["scene_epsilon"]->setFloat( 1.e-4f );

  Variable output_buffer = context["output_buffer"];
  Buffer buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, w, h );

  output_buffer->set(buffer);

  // Ray generation program
  std::string ptx_path( baseDir + "pinhole_camera.cu.ptx" );
  try{
	Program ray_gen_program = context->createProgramFromPTXFile( ptx_path, "pinhole_camera" );
	context->setRayGenerationProgram( 0, ray_gen_program );
  }catch(optix::Exception &e)
  {
      std::string w(e.what());
      std::cout << w << std::endl;
  }

  auto camera = Scene::FirstPersonCamera::getSingleton();
  auto &view = camera->getViewMatrix();
  auto &pos = camera->getPos();
 
  context["eye"]->setFloat( pos.x, pos.y, pos.z );
  context["U"]->setFloat( view[0][0], view[1][0], view[2][0] );
  context["V"]->setFloat( view[0][1], view[1][1], view[2][1] );
  context["W"]->setFloat( view[0][2], view[1][2], view[2][2] );

	/*glm::vec3 W = glm::normalize(glm::vec3(0,-1,-1));          // normalize(at-eye) <--- view direction
	glm::vec3 U = glm::normalize(glm::cross(W, glm::vec3(0,1,0) )); // normalize(w x up) <--- right direction
	glm::vec3 V = glm::normalize(glm::cross(U,W ));           // normalize(u x w)  <--- up direction
	float vLen = tanf( glm::radians( 60.0f * 0.5f ) );   // compute image height <--- tan( fovy / 2)
	float uLen = vLen * 1.0;                      // compute image width  <--- height * aspect

  context["eye"]->setFloat( 0.0f,5.0f,5.0f );
  context["U"]->setFloat(uLen*U.x, uLen*U.y, uLen*U.z);
  context["V"]->setFloat(vLen*V.x, vLen*V.y, vLen*V.z);
  context["W"]->setFloat(W.x,W.y,W.z);*/

  // Exception program
  Program exception_program = context->createProgramFromPTXFile( ptx_path, "exception" );
  context->setExceptionProgram( 0, exception_program );
  context["bad_color"]->setFloat( 1.0f, 1.0f, 0.0f );

  // Miss program
  ptx_path = baseDir + "constantbg.cu.ptx";
  context->setMissProgram( 0, context->createProgramFromPTXFile( ptx_path, "miss" ) );
  context["bg_color"]->setFloat( 0.3f, 0.1f, 0.2f );

  return context;
}


Geometry OptixRender::createGeometry( Context context )
{
  Geometry sphere = context->createGeometry();
  sphere->setPrimitiveCount( 1u );
  sphere->setBoundingBoxProgram( context->createProgramFromPTXFile( baseDir + "sphere.cu.ptx", "bounds" ) );
  sphere->setIntersectionProgram( context->createProgramFromPTXFile( baseDir + "sphere.cu.ptx", "intersect" ) );
  sphere["sphere"]->setFloat( 5, 3, -20.0f, 1.0f );
  return sphere;
}


Material OptixRender::createMaterial( Context context )
{
    Program chp = context->createProgramFromPTXFile( baseDir + "normal_shader.cu.ptx" , "closest_hit_radiance" );

  Material matl = context->createMaterial();
  matl->setClosestHitProgram( 0, chp );
  return matl;
}


void OptixRender::createInstance( Context context, Geometry sphere, Material material )
{
  // Create geometry instance
  GeometryInstance gi = context->createGeometryInstance();
  gi->setMaterialCount( 1 );
  gi->setGeometry( sphere );
  gi->setMaterial( 0, material );

  // Create geometry group
  GeometryGroup geometrygroup = context->createGeometryGroup();
  geometrygroup->setChildCount( 1 );
  geometrygroup->setChild( 0, gi );
  geometrygroup->setAcceleration( context->createAcceleration("NoAccel","NoAccel") );

  context["top_object"]->set( geometrygroup );
}

