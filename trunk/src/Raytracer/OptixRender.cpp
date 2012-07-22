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
	
	// Create a single raygen program
	std::string path_to_ptx = baseDir + "pinhole_camera.cu.ptx";
	optix::Program ray_gen_program = context->createProgramFromPTXFile( path_to_ptx, "pinhole_camera" );
	context->setRayGenerationProgram(0, ray_gen_program);

	unsigned int screenDims[] = {width,height};
	ray_gen_program->declareVariable("rtLaunchDim")->set2uiv( screenDims );

	// Create miss program. might be necessary
	path_to_ptx = baseDir + "constantbg.cu.ptx";
	optix::Program miss_program = context->createProgramFromPTXFile( path_to_ptx, "miss" );
	context->setMissProgram(0, miss_program );

	g_buffer_diffuse[PBO_READ] = new optix::Buffer;
	g_buffer_diffuse[PBO_WRITE] = new optix::Buffer;
	g_buffer_position[PBO_READ] = new optix::Buffer;
	g_buffer_position[PBO_WRITE] = new optix::Buffer;
	g_buffer_normal[PBO_READ] = new optix::Buffer;
	g_buffer_normal[PBO_WRITE] = new optix::Buffer;
	createGBuffers();
	
	// let Raytracer read from Raster Gbuffer
	context["g_buffer_diffuse_read"]->set(*g_buffer_diffuse[PBO_READ]);
	context["g_buffer_position_read"]->set(*g_buffer_position[PBO_READ]);
	context["g_buffer_normal_read"]->set(*g_buffer_normal[PBO_READ]);

	// let Raytracer write to its own Gbuffer (to be consumed by GLSL/Raster)
	context["g_buffer_diffuse_write"]->set(*g_buffer_diffuse[PBO_WRITE]);
	context["g_buffer_position_write"]->set(*g_buffer_position[PBO_WRITE]);
	context["g_buffer_normal_write"]->set(*g_buffer_normal[PBO_WRITE]);
}

void OptixRender::compileContext()
{
	// First Run, trap any exceptions before mainloop
	try{
		context->validate();
		context->compile();
		context->launch( 0, width, height);
	}catch(const optix::Exception &e){
		std::cout << e.getErrorString();
		system("pause");
		exit(1);
	}
}

optix::Context OptixRender::minimalCreateContext()
{
	Context context = Context::create();
	context->setRayTypeCount( 1 ); // radiance only for now
	context->setEntryPointCount( 1 );

	return context;
}

// adapted from suti: optix\SDK\sutil\SampleScene.cpp
void OptixRender::createGBuffers()
{
	size_t element_size = 0;

	//DIFFUSE BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_UNSIGNED_BYTE4, &element_size) );
	g_buffer_diffuse_pbo[PBO_READ] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_diffuse_pbo[PBO_READ]->unbind();
	*g_buffer_diffuse[PBO_READ] = context->createBufferFromGLBO(RT_BUFFER_INPUT, g_buffer_diffuse_pbo[PBO_READ]->getHandle());
	(*g_buffer_diffuse[PBO_READ])->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	(*g_buffer_diffuse[PBO_READ])->setSize( width, height );

	//POSITION BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_position_pbo[PBO_READ] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_position_pbo[PBO_READ]->unbind();
	*g_buffer_position[PBO_READ] = context->createBufferFromGLBO(RT_BUFFER_INPUT, g_buffer_position_pbo[PBO_READ]->getHandle());
	(*g_buffer_position[PBO_READ])->setFormat(RT_FORMAT_FLOAT4);
	(*g_buffer_position[PBO_READ])->setSize( width, height );

	//NORMAL BUFFER
	//context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_normal_pbo[PBO_READ] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_normal_pbo[PBO_READ]->unbind();
	*g_buffer_normal[PBO_READ] = context->createBufferFromGLBO(RT_BUFFER_INPUT, g_buffer_normal_pbo[PBO_READ]->getHandle());
	(*g_buffer_normal[PBO_READ])->setFormat(RT_FORMAT_FLOAT4);
	(*g_buffer_normal[PBO_READ])->setSize( width, height );

	// Write
	//DIFFUSE BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_UNSIGNED_BYTE4, &element_size) );
	g_buffer_diffuse_pbo[PBO_WRITE] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_diffuse_pbo[PBO_WRITE]->unbind();
	*g_buffer_diffuse[PBO_WRITE] = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, g_buffer_diffuse_pbo[PBO_WRITE]->getHandle());
	(*g_buffer_diffuse[PBO_WRITE])->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	(*g_buffer_diffuse[PBO_WRITE])->setSize( width, height );

	//POSITION BUFFER
	context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_position_pbo[PBO_WRITE] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_position_pbo[PBO_WRITE]->unbind();
	*g_buffer_position[PBO_WRITE] = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, g_buffer_position_pbo[PBO_WRITE]->getHandle());
	(*g_buffer_position[PBO_WRITE])->setFormat(RT_FORMAT_FLOAT4);
	(*g_buffer_position[PBO_WRITE])->setSize( width, height );

	//NORMAL BUFFER
	//context->checkError( rtuGetSizeForRTformat(RT_FORMAT_FLOAT4, &element_size) );
	g_buffer_normal_pbo[PBO_WRITE] = std::make_shared<Render::PBO>(element_size * width * height, GL_STREAM_DRAW, true);
	g_buffer_normal_pbo[PBO_WRITE]->unbind();
	*g_buffer_normal[PBO_WRITE] = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, g_buffer_normal_pbo[PBO_WRITE]->getHandle());
	(*g_buffer_normal[PBO_WRITE])->setFormat(RT_FORMAT_FLOAT4);
	(*g_buffer_normal[PBO_WRITE])->setSize( width, height );


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
	g_buffer_diffuse_pbo[PBO_READ]->bufferFromTextureOnGPU(raster_diffuse, 0);
	g_buffer_position_pbo[PBO_READ]->bufferFromTextureOnGPU(raster_position, 0);
	g_buffer_normal_pbo[PBO_READ]->bufferFromTextureOnGPU(raster_normal, 0);

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

// Take G-buffer generated in Raytracer, convert to GLSL readable
void OptixRender::pbo2Texture()
{
	auto raster_fbo = g_buffer_pass->getFBO();
	auto raster_diffuse = raster_fbo->getRenderTexture(0);
	auto raster_position = raster_fbo->getRenderTexture(1);
	auto raster_normal = raster_fbo->getRenderTexture(2);

	g_buffer_diffuse_pbo[PBO_WRITE]->bind();
	{
		g_buffer_diffuse_pbo[PBO_WRITE]->align(getBufferAlignment(*g_buffer_diffuse[PBO_WRITE]));
		g_buffer_diffuse_pbo[PBO_WRITE]->copyToTextureOnGPU(raster_diffuse, 0);
	} g_buffer_diffuse_pbo[PBO_WRITE]->unbind();

	g_buffer_position_pbo[PBO_WRITE]->bind();
	{
		g_buffer_position_pbo[PBO_WRITE]->align(getBufferAlignment(*g_buffer_position[PBO_WRITE]));
		g_buffer_position_pbo[PBO_WRITE]->copyToTextureOnGPU(raster_position, 0);
	} g_buffer_position_pbo[PBO_WRITE]->unbind();

	g_buffer_normal_pbo[PBO_WRITE]->bind();
	{
		g_buffer_normal_pbo[PBO_WRITE]->align(getBufferAlignment(*g_buffer_normal[PBO_WRITE]));
		g_buffer_normal_pbo[PBO_WRITE]->copyToTextureOnGPU(raster_normal, 0);
	} g_buffer_normal_pbo[PBO_WRITE]->unbind();
} 