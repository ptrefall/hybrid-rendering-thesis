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
    context = minimalCreateContext();

	optix::Program ray_gen_program = context->createProgramFromPTXFile( baseDir + "g_buffer.cu.ptx", "gbuffer_compose" );
	context->setRayGenerationProgram( 0, ray_gen_program );
	unsigned int screenDims[] = {width,height};
	ray_gen_program->declareVariable("rtLaunchDim")->set2uiv( screenDims );
	createGBuffers();
	context["g_buffer_diffuse"]->set(g_buffer_diffuse);
	context["g_buffer_position"]->set(g_buffer_position);
	context["g_buffer_normal"]->set(g_buffer_normal);
    
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
void OptixRender::createGBuffers()
{
	size_t element_size = 0;

	//DIFFUSE BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_UNSIGNED_BYTE4, &element_size) );
	g_buffer_diffuse_pbo = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_diffuse_pbo->unbind();
	g_buffer_diffuse = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, g_buffer_diffuse_pbo->getHandle());
	g_buffer_diffuse->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	g_buffer_diffuse->setSize( width, height );

	//POSITION BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_position_pbo = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_position_pbo->unbind();
	g_buffer_position = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, g_buffer_position_pbo->getHandle());
	g_buffer_position->setFormat(RT_FORMAT_FLOAT4);
	g_buffer_position->setSize( width, height );

	//NORMAL BUFFER
	//context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_normal_pbo = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_normal_pbo->unbind();
	g_buffer_normal = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, g_buffer_normal_pbo->getHandle());
	g_buffer_normal->setFormat(RT_FORMAT_FLOAT4);
	g_buffer_normal->setSize( width, height );

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
}


void OptixRender::render()
{
	auto camera = Scene::FirstPersonCamera::getSingleton();
	auto &world_to_view = camera->getWorldToViewMatrix();
	
	// Calculate camera vectors
	{
		float aspect = width/(float)height;
		float vfov = camera->getFovDegrees() * (3.14f/180.f);
		float screenDist = 1.0 / tan(vfov * 0.5);

		glm::vec3 eyePos = camera->getPos();
		glm::vec3 u = aspect * camera->getStrafeDirection();
		glm::vec3 v = camera->getUpDirection();
		glm::vec3 w = screenDist * camera->getLookDirection();

		context["eye"]->set3fv( glm::value_ptr(eyePos) );
		context["U"]->set3fv( glm::value_ptr(u) );
		context["V"]->set3fv( glm::value_ptr(v) );
		context["W"]->set3fv( glm::value_ptr(w) );
	}

	//Upload rasterized g-buffer info here:
	auto raster_fbo = g_buffer_pass->getFBO();
	auto raster_diffuse = raster_fbo->getRenderTexture(0);
	auto raster_position = raster_fbo->getRenderTexture(1);
	auto raster_normal = raster_fbo->getRenderTexture(2);
	g_buffer_diffuse_pbo->bufferFromTextureOnGPU(raster_diffuse, 0);
	g_buffer_position_pbo->bufferFromTextureOnGPU(raster_position, 0);
	g_buffer_normal_pbo->bufferFromTextureOnGPU(raster_normal, 0);

	try {
		context->launch(0, width,height);
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

unsigned int OptixRender::getBufferAlignment(optix::Buffer buffer)
{
	RTsize elementSize = buffer->getElementSize();
    if      ((elementSize % 8) == 0) return 8;
    else if ((elementSize % 4) == 0) return 4;
    else if ((elementSize % 2) == 0) return 2;
    else                             return 1;
}

void OptixRender::pbo2Texture()
{
	auto raster_fbo = g_buffer_pass->getFBO();
	auto raster_diffuse = raster_fbo->getRenderTexture(0);
	auto raster_position = raster_fbo->getRenderTexture(1);
	auto raster_normal = raster_fbo->getRenderTexture(2);

	g_buffer_diffuse_pbo->bind();
	{
		g_buffer_diffuse_pbo->align(getBufferAlignment(g_buffer_diffuse));
		g_buffer_diffuse_pbo->copyToTextureOnGPU(raster_diffuse, 0);
	} g_buffer_diffuse_pbo->unbind();

	g_buffer_position_pbo->bind();
	{
		g_buffer_position_pbo->align(getBufferAlignment(g_buffer_position));
		g_buffer_position_pbo->copyToTextureOnGPU(raster_position, 0);
	} g_buffer_position_pbo->unbind();

	g_buffer_normal_pbo->bind();
	{
		g_buffer_normal_pbo->align(getBufferAlignment(g_buffer_normal));
		g_buffer_normal_pbo->copyToTextureOnGPU(raster_normal, 0);
	} g_buffer_normal_pbo->unbind();
} 