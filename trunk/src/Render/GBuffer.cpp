#include "GBuffer.h"

#include "RT.h"
#include "Tex2D.h"

#include "ShaderConstants.h"

using namespace Render;

GBuffer::GBuffer(const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h) 
	: shader_loader(shader_loader), w(w), h(h)
{
	////////////////////////////////////////
	// LOAD FBO
	////////////////////////////////////////
	fbo = std::make_shared<FBO>(w,h);
	
	//Add render targets
	fbo->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse
	fbo->add(GL_COLOR_ATTACHMENT1, std::make_shared<RT>(GL_RGBA32F, w,h));				//Position
	fbo->add(GL_COLOR_ATTACHMENT2, std::make_shared<RT>(GL_RGBA16F, w,h));				//Normal
	fbo->add(GL_DEPTH_ATTACHMENT,  std::make_shared<RT>(GL_DEPTH_COMPONENT24, w,h));	//Depth

	//Add render textures
	fbo->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, w,h)));//Diffuse
	fbo->add(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, "TEX_POS",  std::make_shared<Tex2D>(T2DTexParams(GL_RGBA32F, GL_RGBA, GL_FLOAT, w,h)));		//Position
	fbo->add(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, "TEX_NORM", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, w,h)));		//Normal

	//Check that everything is ok
	fbo->check();
	fbo->unbind();

	////////////////////////////////////////
	// LOAD SHADER
	////////////////////////////////////////
	shader = shader_loader->load("deferredShading.vs", std::string(), "deferredShading.fs");
	mvp		= std::make_shared<Uniform>(shader->getVS(), "MVP");
	mv		= std::make_shared<Uniform>(shader->getVS(), "MV");
	n_wri	= std::make_shared<Uniform>(shader->getVS(), "N_WRI");
}

void GBuffer::begin()
{
	shader->bind();
	fbo->bind();

	GLint nViewport[4];
	glGetIntegerv(GL_VIEWPORT, nViewport);
	temp_w = nViewport[2];
	temp_h = nViewport[3];
	glViewportIndexedf(0,0,0,(float)w,(float)h);

	float color_buffer_clear[4];
	memset(color_buffer_clear, 0, sizeof(float)*4);
	float depth_buffer_clear = 0.0f;
	glClearBufferfv(GL_COLOR, 0, color_buffer_clear);
	glClearBufferfv(GL_DEPTH, 0, &depth_buffer_clear);

	glActiveTexture(GL_TEXTURE0);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);
}

void GBuffer::end()
{
	shader->unbind();
	fbo->unbind();
	glViewportIndexedf(0,0,0,(float)temp_w,(float)temp_h);
}

void GBuffer::bind(unsigned int active_program)
{
	fbo->bind_rt(active_program);
}

void GBuffer::unbind()
{
	fbo->unbind_rt();
}

void GBuffer::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
