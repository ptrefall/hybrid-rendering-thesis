#include "GBuffer.h"

#include "RT.h"
#include "Tex2D.h"

using namespace Render;

GBuffer::GBuffer() 
	: w(800), h(600)
{
	fbo = std::make_shared<FBO>(w,h);
	
	//Add render targets
	fbo->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse
	fbo->add(GL_COLOR_ATTACHMENT1, std::make_shared<RT>(GL_RGBA32F, w,h));				//Position
	fbo->add(GL_COLOR_ATTACHMENT2, std::make_shared<RT>(GL_RGBA16F, w,h));				//Normal
	fbo->add(GL_DEPTH_ATTACHMENT,  std::make_shared<RT>(GL_DEPTH_COMPONENT24, w,h));	//Depth

	//Add render textures
	fbo->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, std::make_shared<Tex2D>(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, w,h));//Diffuse
	fbo->add(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, std::make_shared<Tex2D>(GL_RGBA32F, GL_RGBA, GL_FLOAT, w,h));		//Position
	fbo->add(GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, std::make_shared<Tex2D>(GL_RGBA16F, GL_RGBA, GL_FLOAT, w,h));		//Normal

	//Check that everything is ok
	fbo->check();
	fbo->unbind();
}

void GBuffer::begin()
{
	fbo->bind();

	GLint nViewport[4];
    glGetIntegerv(GL_VIEWPORT, nViewport);
	temp_w = nViewport[2];
	temp_h = nViewport[3];
	glViewportIndexedf(0,0,0,(float)w,(float)h);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);
}

void GBuffer::end()
{
	fbo->unbind();
	glViewportIndexedf(0,0,0,(float)temp_w,(float)temp_h);
}

void GBuffer::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
