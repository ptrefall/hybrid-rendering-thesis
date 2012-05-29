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
	fbo->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, w,h)));//Diffuse
	fbo->add(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, "TEX_POS",  std::make_shared<Tex2D>(T2DTexParams(GL_RGBA32F, GL_RGBA, GL_FLOAT, 4, w,h)));		//Position
	fbo->add(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, "TEX_NORM", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, 4, w,h)));		//Normal

	//Check that everything is ok
	fbo->check();
	fbo->unbind();

	////////////////////////////////////////
	// LOAD SHADER
	////////////////////////////////////////
	shader = shader_loader->load("deferredShading.vs", std::string(), "deferredShading.fs");
	uni_object_to_world		= std::make_shared<Render::Uniform>(shader->getVS(), "Object_to_World");
	uni_world_to_view		= std::make_shared<Render::Uniform>(shader->getVS(), "World_to_View");
	uni_view_to_clip		= std::make_shared<Render::Uniform>(shader->getVS(), "View_to_Clip");
	uni_normal_to_view		= std::make_shared<Render::Uniform>(shader->getVS(), "Normal_to_View");
}

void GBuffer::begin()
{
    glClearColor(0.f,1.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//shader->bind();
	shader_loader->push_bind(shader);
	fbo->bind();

    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	GLint nViewport[4];
	glGetIntegerv(GL_VIEWPORT, nViewport);
	temp_w = nViewport[2];
	temp_h = nViewport[3];
	glViewportIndexedf(0,0,0,(float)w,(float)h);
	//glViewport(0,0,w,h);
	glActiveTexture(GL_TEXTURE0);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(3, buffers);
}

void GBuffer::end()
{
	//shader->unbind();
	shader_loader->pop_bind();
	fbo->unbind();
	glViewportIndexedf(0,0,0,(float)temp_w,(float)temp_h);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
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
