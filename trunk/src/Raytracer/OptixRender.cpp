#include "OptixRender.h"
#include "../Scene/proto_camera.h"
//#include "../Kernel.h"
//#include "../File/TextureLoader.h"

#include "../Render/Passes/GBuffer/GBuffer_Pass.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

#include <Optix/optixu/optixu.h>

using namespace Raytracer;
using namespace optix;

OptixRender::OptixRender(const Render::GBuffer_PassPtr &g_buffer_pass, unsigned int width, unsigned int height, const std::string& baseDir)
	: g_buffer_pass(g_buffer_pass), width(width), height(height), baseDir(baseDir)
{
    GLenum error = glGetError();
	tex = std::make_shared<Render::Tex2D>();
    error = glGetError();
	Render::T2DTexParams params((unsigned int)GL_BGRA, (unsigned int)GL_BGRA, (unsigned int)GL_UNSIGNED_BYTE, 4, (unsigned int)width, (unsigned int)height, (unsigned int)GL_CLAMP_TO_EDGE, (unsigned char*)nullptr);
    error = glGetError();
	tex->update(params);
    error = glGetError();

    context = minimalCreateContext();

	optix::Program ray_gen_program = context->createProgramFromPTXFile( baseDir + "g_buffer.cu.ptx", "gbuffer_compose" );
	context->setRayGenerationProgram( 0, ray_gen_program );
	unsigned int screenDims[] = {width,height};
	ray_gen_program->declareVariable("rtLaunchDim")->set2uiv( screenDims );
	g_buffer = createGBuffer();
	//ray_gen_program["g_buffer"]->set(g_buffer); // Why must this var be set on a RayGen (fails on context) ?
	context["g_buffer"]->set(g_buffer);
    
    // First Run, trap any exceptions before mainloop
	try{
		context->validate();
		context->compile();
		context->launch( 0, width, height);
	}catch(const optix::Exception &e){
		std::cout << e.getErrorString();
	}
}

optix::Context OptixRender::minimalCreateContext()
{
	Context context = Context::create();
	context->setRayTypeCount( 1 );
	context->setEntryPointCount( 1 );

	return context;
}

// adapted from suti: optix\SDK\sutil\SampleScene.cpp
optix::Buffer OptixRender::createGBuffer()
{
	Buffer buffer;
	RTformat format = RT_FORMAT_UNSIGNED_BYTE4;
	size_t element_size = 0;
	context->checkError( rtuGetSizeForRTformat(format, &element_size) );

	//Upload rasterized g-buffer info here:
	auto raster_fbo = g_buffer_pass->getFBO();
	auto raster_diffuse = raster_fbo->getRenderTexture(0);
	auto raster_position = raster_fbo->getRenderTexture(1);
	auto raster_normal = raster_fbo->getRenderTexture(2);
	raster_diffuse->bind();

	/*
		Allocate first the memory for the gl buffer, then attach it to OptiX.
	*/
	//TODO: Size of PBO should be equal to number of render textures in gbuffer_pass' fbo, and also equal to their sizes and types...
	g_buffer_pbo = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_pbo->unbind();

	buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, g_buffer_pbo->getHandle());
	buffer->setFormat(format);
	buffer->setSize( width, height );

	// Set number of devices to be used
	// Default, 0, means not to specify them here, but let OptiX use its default behavior.
	int _num_devices = 0;
	if(_num_devices) 
	{
		int max_num_devices    = Context::getDeviceCount();
		int actual_num_devices = std::min( max_num_devices, std::max( 1, _num_devices ) );
		std::vector<int> devs(actual_num_devices);
		for( int i = 0; i < actual_num_devices; ++i ) devs[i] = i;
			context->setDevices( devs.begin(), devs.end() );
	}

	return buffer;
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
  //unsigned int pbo = context["output_buffer"]->getBuffer()->getGLBOId(); // is pbo now.
	auto camera = Scene::FirstPersonCamera::getSingleton();
	auto &world_to_view = camera->getWorldToViewMatrix();
	auto pos = camera->getPos();
 
    float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    float inv_aspect_ratio = static_cast<float>(height) / static_cast<float>(width);
    
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
		context->launch(0, width,height);
	} catch (optix::Exception& e) {
		std::cout << e.getErrorString();
		return;
	}

	pbo2Texture();
}

void OptixRender::reshape(unsigned int width, unsigned int height) 
{ 
	this->width = width;
	this->height = height; 
}

Context OptixRender::createContext() 
{
  // Set up context
  Context context = Context::create();
  context->setRayTypeCount( 1 );
  context->setEntryPointCount( 1 );
  context->setStackSize( 400 );

  context["radiance_ray_type"]->setUint( 0u );
  context["scene_epsilon"]->setFloat( 1.e-4f );

  Variable output_buffer = context["output_buffer"];
  Buffer buffer = context->createBuffer( RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height );

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
  Geometry dummy = context->createGeometry();
  dummy->setPrimitiveCount( 1u );
  dummy->setBoundingBoxProgram( context->createProgramFromPTXFile( baseDir + "g_buffer.cu.ptx", "bounds" ) );
  dummy->setIntersectionProgram( context->createProgramFromPTXFile( baseDir + "g_buffer.cu.ptx", "intersect" ) );
  //dummy["dummy"]->setFloat( 0.f, 0.f, 0.f, 1.0f );
  dummy["dummy"]->setFloat( 5, 3, -20.0f, 1.0f );
  return dummy;
}


Material OptixRender::createMaterial( Context context )
{
    //Program chp = context->createProgramFromPTXFile( baseDir + "normal_shader.cu.ptx" , "closest_hit_radiance" );
	Program chp = context->createProgramFromPTXFile( baseDir + "g_buffer.cu.ptx", "closest_hit_radiance" );
	Material matl = context->createMaterial();
	matl->setClosestHitProgram( 0, chp );
	return matl;
}


void OptixRender::createInstance( Context context, Geometry dummy, Material material )
{
  // Create geometry instance
  GeometryInstance gi = context->createGeometryInstance();
  gi->setMaterialCount( 1 );
  gi->setGeometry( dummy );
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
	addTextureSampler(raster_diffuse_sampler, raster_diffuse->getHandle(), 1.0f, "raster_diffuse_tex");
	addTextureSampler(raster_position_sampler, raster_position->getHandle(), 1.0f, "raster_position_tex");
	addTextureSampler(raster_normal_sampler, raster_normal->getHandle(), 1.0f, "raster_normal_tex");
}

void OptixRender::addTextureSampler(optix::TextureSampler sampler, unsigned int gl_tex_handle, float max_anisotropy, const std::string &sampler_name)
{
	sampler = context->createTextureSamplerFromGLImage( gl_tex_handle, RT_TARGET_GL_TEXTURE_2D );
	sampler->setWrapMode( 0, RT_WRAP_CLAMP_TO_EDGE );
	sampler->setWrapMode( 1, RT_WRAP_CLAMP_TO_EDGE );
	sampler->setWrapMode( 2, RT_WRAP_CLAMP_TO_EDGE );
	sampler->setIndexingMode( RT_TEXTURE_INDEX_NORMALIZED_COORDINATES );
	sampler->setReadMode( RT_TEXTURE_READ_NORMALIZED_FLOAT );
	sampler->setMaxAnisotropy( max_anisotropy );
	sampler->setFilteringModes( RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE );

	context[sampler_name.c_str()]->setTextureSampler( sampler );
}

void OptixRender::pbo2Texture()
{
	g_buffer_pbo->bind();
	tex->bind();

    RTsize elementSize = g_buffer->getElementSize();
    if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	RTsize buffer_width_rts, buffer_height_rts;
	g_buffer->getSize( buffer_width_rts, buffer_height_rts );
	int buffer_width  = static_cast<int>(buffer_width_rts);
	int buffer_height = static_cast<int>(buffer_height_rts);
	RTformat buffer_format = g_buffer->getFormat();

	//TODO: Change this to respect the 3 textures of the g_buffer, and offset the glTexImage2D lookup from the PBO correctly..

    if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer_width, buffer_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    } else if(buffer_format == RT_FORMAT_FLOAT4) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
    } else if(buffer_format == RT_FORMAT_FLOAT3) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, buffer_width, buffer_height, 0, GL_RGB, GL_FLOAT, 0);
    } else {
      assert(0 && "Unknown buffer format");
    }

    g_buffer_pbo->unbind();
}