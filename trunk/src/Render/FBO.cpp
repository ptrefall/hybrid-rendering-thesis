#include "FBO.h"
//#include "../Kernel.h"
//#include "../File/TextureLoader.h"
using namespace Render;

// TODO fbo doesn't actually use members w,h yet
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

void FBO::bind_rt(unsigned int active_program, unsigned int index_offset)
{
	for(auto i = 0; i < (int)render_textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i+index_offset);
		render_textures[i]->bind();
		render_samplers[i]->bind(i+1, active_program);
		//Kernel::getSingleton()->getTextureLoader()->save(render_textures[i], Kernel::getSingleton()->getResourceDir()+"screens\\MRT.png");
	}
}

void FBO::unbind_rt()
{
	for(unsigned int i = 0; i < render_textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		render_textures[i]->unbind();
	}
}

void FBO::add(unsigned int attachment, const RTPtr &render_target)
{
	render_targets.push_back(render_target);

	render_target->bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, render_target->getHandle());
}

void FBO::add(unsigned int attachment, unsigned int texture_type, const std::string &sampler_name, const Tex2DPtr &render_texture)
{
	render_textures.push_back(render_texture);
	render_samplers.push_back(std::make_shared<Uniform>(sampler_name));

	render_texture->bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture_type, render_texture->getHandle(), 0);
}

void FBO::check()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Can't initialize an FBO render texture. FBO initialization failed.");
}
