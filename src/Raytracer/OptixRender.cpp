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
	diffuse_tex = std::make_shared<Render::Tex2D>();
	Render::T2DTexParams params((unsigned int)GL_RGBA8, (unsigned int)GL_RGBA, (unsigned int)GL_UNSIGNED_BYTE, 4, (unsigned int)width, (unsigned int)height, (unsigned int)GL_CLAMP_TO_EDGE, (unsigned char*)nullptr);
	diffuse_tex->update(params); 

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

	/*
		Allocate first the memory for the gl buffer, then attach it to OptiX.
	*/
	//TODO: Size of PBO should be equal to number of render textures in gbuffer_pass' fbo, and also equal to their sizes and types...
	g_buffer_pbo = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_pbo->unbind();
	//g_buffer_pbo->bufferFromTextureOnGPU(raster_diffuse, 0, GL_STREAM_DRAW);

	buffer = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, g_buffer_pbo->getHandle());
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

	//auto gl_id_handle = g_buffer->getGLBOId();
	//static bool glbuffer_registered = true;
	try {
		//if(!glbuffer_registered)
		//{
		//	g_buffer->registerGLBuffer();
		//	glbuffer_registered = true;
		//}
		context->launch(0, width,height);
		//g_buffer->unregisterGLBuffer();
		//glbuffer_registered = false;
	} catch (optix::Exception& e) {
		std::cout << e.getErrorString();
		return; 
	}
	 
	glActiveTexture(GL_TEXTURE0); 
	pbo2Texture();
}

void OptixRender::reshape(unsigned int width, unsigned int height) 
{ 
	this->width = width;
	this->height = height; 
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

    RTsize elementSize = g_buffer->getElementSize();
    if      ((elementSize % 8) == 0) g_buffer_pbo->align(8);
    else if ((elementSize % 4) == 0) g_buffer_pbo->align(4);
    else if ((elementSize % 2) == 0) g_buffer_pbo->align(2);
    else                             g_buffer_pbo->align(1);

	//TODO: Change this to respect the 3 textures of the g_buffer, and offset the glTexImage2D lookup from the PBO correctly..
	unsigned int diffuse_offset = 0;
    unsigned int position_offset =	g_buffer_pbo->copyToTextureOnGPU(diffuse_tex, diffuse_offset);
	//unsigned int normal_offset =	g_buffer_pbo->copyToTextureOnGPU(position_tex, position_offset);
	//								g_buffer_pbo->copyToTextureOnGPU(normal_tex, normal_offset);

    g_buffer_pbo->unbind();
} 