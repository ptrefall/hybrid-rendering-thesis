#include "OptixRender.h"
#include "../Scene/proto_camera.h"
//#include "../Kernel.h"
//#include "../File/TextureLoader.h"

#include "../Render/Passes/GBuffer/GBuffer_Pass.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

using namespace Raytracer;
using namespace optix;

OptixRender::OptixRender(const Render::GBuffer_PassPtr &g_buffer_pass, unsigned int w, unsigned int h, const std::string& baseDir)
	: g_buffer_pass(g_buffer_pass), w(w), h(h), baseDir(baseDir)
{
    GLenum error = glGetError();
	tex = std::make_shared<Render::Tex2D>();
    error = glGetError();
    Render::T2DTexParams params((unsigned int)GL_BGRA, (unsigned int)GL_BGRA, (unsigned int)GL_UNSIGNED_BYTE, 4, (unsigned int)w, (unsigned int)h, (unsigned int)GL_CLAMP_TO_EDGE, (unsigned char*)nullptr);
    error = glGetError();
	tex->update(params);
    error = glGetError();

    context  = createContext();
    sphere   = createGeometry( context );
    material = createMaterial( context );
    createInstance( context, sphere, material );
	//createTextureSamplers( context );
    
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

  auto buf = (unsigned char*)buffer->map();
  std::vector<unsigned char> imageData(buf, buf + buffer_width*buffer_height*4*sizeof(unsigned char));
  
    //assert( &imageData[0] );

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
    
	Render::T2DTexParams params((unsigned int)gl_format, (unsigned int)gl_format, (unsigned int)gl_data_type, 4, (unsigned int)buffer_width, (unsigned int)buffer_height, (unsigned int)GL_CLAMP_TO_EDGE, (unsigned char*)&imageData[0]);
	tex->update(params);

    

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

// C++ version of sutil.c code
void CalculateCameraVariables(  glm::vec3 eye, glm::vec3 lookAt, glm::vec3 up,
                                float hfov,
                                float aspect_ratio,
                                glm::vec3& U,
                                glm::vec3& V,
                                glm::vec3& W )
{
  
  W = lookAt - eye;  /* Do not normalize W -- it implies focal length */
  U = glm::normalize( glm::cross(W, up) );
  V = glm::normalize( glm::cross( U, W ) );
  float wlen = sqrtf( glm::dot( W, W ) );
  float ulen = wlen * tanf( hfov / 2.0f * 3.14159265358979323846f / 180.0f );
  float vlen =  ulen/aspect_ratio;
  U *= ulen;
  V *= vlen;
}

void CalculateCameraVariables(  float hfov,
                                float aspect_ratio,
                                glm::vec3& U,
                                glm::vec3& V,
                                glm::vec3& W )
{
    auto &cam = Scene::FirstPersonCamera::getSingleton();
    glm::vec3 side = cam->getStrafeDirection();
    glm::vec3 up = cam->getUpDirection();
    glm::vec3 fwd = cam->getLookDirection();

    // set up the coordinate system
    W = fwd;//glm::normalize (fwd); 
    U = side;//glm::normalize (glm::cross(up, W));
    V = up;//glm::normalize (glm::cross(W, U));

    float wlen = sqrtf( glm::dot( W, W ) );
    float ulen = wlen * tanf( glm::radians(hfov) / 2.0f );
    U *= ulen;
    float vlen = ulen/aspect_ratio;
    V *= vlen;
}

void OptixRender::render()
{
  //unsigned int pbo = context["output_buffer"]->getBuffer()->getGLBOId(); // not a pbo!
	auto camera = Scene::FirstPersonCamera::getSingleton();
	auto &world_to_view = camera->getWorldToViewMatrix();
	auto pos = camera->getPos();
 
    float aspect_ratio = static_cast<float>(w) / static_cast<float>(h);
    float inv_aspect_ratio = static_cast<float>(h) / static_cast<float>(w);
    
    float vfov = camera->getFov();
    float hfov = vfov * aspect_ratio;

    glm::vec3 U,V,W;
    //CalculateCameraVariables( hfov, aspect_ratio, U, V, W );
    CalculateCameraVariables( pos, pos+camera->getLookDirection(), camera->getUpDirection(), hfov, aspect_ratio, U, V, W );

    context["eye"]->set3fv( glm::value_ptr(pos) );
    context["U"]->set3fv( glm::value_ptr(U) );
    context["V"]->set3fv( glm::value_ptr(V) );
    context["W"]->set3fv( glm::value_ptr(W) );

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
  //sphere["sphere"]->setFloat( 0.f, 0.f, 0.f, 1.0f );
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

void OptixRender::createTextureSamplers( optix::Context context )
{
	//Upload rasterized g-buffer info here:
	auto raster_fbo = g_buffer_pass->getFBO();
	auto raster_diffuse = raster_fbo->getRenderTexture(0);
	auto raster_position = raster_fbo->getRenderTexture(1);
	auto raster_normal = raster_fbo->getRenderTexture(2);
	//TODO: Upload to optix!
	addTextureSampler(raster_diffuse_sampler, raster_diffuse->getHandle(), 1.0f, "raster_diffuse");
	addTextureSampler(raster_position_sampler, raster_position->getHandle(), 1.0f, "raster_position");
	addTextureSampler(raster_normal_sampler, raster_normal->getHandle(), 1.0f, "raster_normal");
}

void OptixRender::addTextureSampler(optix::TextureSampler sampler, unsigned int gl_tex_handle, float max_anisotropy, const std::string &sampler_name)
{
	sampler = context->createTextureSamplerFromGLImage( gl_tex_handle, RT_TARGET_GL_TEXTURE_2D );
	sampler->setWrapMode( 0, RT_WRAP_REPEAT );
	sampler->setWrapMode( 1, RT_WRAP_REPEAT );
	sampler->setWrapMode( 2, RT_WRAP_REPEAT );
	sampler->setIndexingMode( RT_TEXTURE_INDEX_NORMALIZED_COORDINATES );
	sampler->setReadMode( RT_TEXTURE_READ_NORMALIZED_FLOAT );
	sampler->setMaxAnisotropy( max_anisotropy );
	sampler->setFilteringModes( RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE );

	context[sampler_name.c_str()]->setTextureSampler( sampler );
}
