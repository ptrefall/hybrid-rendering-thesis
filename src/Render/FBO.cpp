#include "FBO.h"

using namespace Render;

FBO::FBO(unsigned int w, unsigned int h)
	: w(w), h(h)
{
	glGenFramebuffers(1, &handle);
	bind();
}

FBO::~FBO()
{
	glDeleteFramebuffers(1, &handle);
}

void FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::add(unsigned int attachment, const RTPtr &render_target)
{
	render_targets.push_back(render_target);

	render_target->bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, render_target->getHandle());
}

void FBO::add(unsigned int attachment, unsigned int texture_type, const Tex2DPtr &render_texture)
{
	render_textures.push_back(render_texture);

	render_texture->bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, texture_type, render_texture->getHandle());
}

void FBO::check()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Can't initialize an FBO render texture. FBO initialization failed.");
}
