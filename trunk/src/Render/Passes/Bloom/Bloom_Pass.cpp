#include "Bloom_Pass.h"

#include "../Final/Final_Pass.h"
#include "../../../Scene/Camera.h"

using namespace Render;

Bloom_Pass::Bloom_Pass(const Final_PassPtr &final_pass, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h)
	: final_pass(final_pass), shader_loader(shader_loader), w(w), h(h)
{
	// EXTRACTION FBO
	fbo_extraction = std::make_shared<FBO>(w,h);
	fbo_extraction->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse
	fbo_extraction->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, w,h)));//Diffuse
	fbo_extraction->check();
	fbo_extraction->unbind();

	// BLUR VERTICAL FBO
	fbo_blur_vertical = std::make_shared<FBO>(w,h);
	fbo_blur_vertical->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse
	fbo_blur_vertical->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, w,h)));//Diffuse
	fbo_blur_vertical->check();
	fbo_blur_vertical->unbind();

	// BLUR HORIZONTAL FBO
	fbo_blur_horizontal = std::make_shared<FBO>(w,h);
	fbo_blur_horizontal->add(GL_COLOR_ATTACHMENT0, std::make_shared<RT>(GL_RGBA, w,h));					//Diffuse
	fbo_blur_horizontal->add(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, "TEX_DIFF", std::make_shared<Tex2D>(T2DTexParams(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, w,h)));//Diffuse
	fbo_blur_horizontal->check();
	fbo_blur_horizontal->unbind();

	quad = std::make_shared<Scene::Quad>(w,h);
	shader_extraction = shader_loader->load("present.vs", std::string(), "bloom_pass_extraction.fs");
	shader_blur_vertical = shader_loader->load("present.vs", std::string(), "bloom_pass_blur_vertical.fs");
	shader_blur_horizontal = shader_loader->load("present.vs", std::string(), "bloom_pass_blur_horizontal.fs");
	shader_final = shader_loader->load("present.vs", std::string(), "bloom_pass_final.fs");
}

void Bloom_Pass::begin()
{
}

void Bloom_Pass::render_extraction_step()
{
	fbo_extraction->bind();
	{
		glClearColor(0.f,0.f,0.f,1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		shader_loader->push_bind(shader_extraction);
		final_pass->bind(shader_extraction->getFS());
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		quad->render(0);
		final_pass->unbind();
		shader_loader->pop_bind();
	} fbo_extraction->unbind();
}

void Bloom_Pass::render_blur_steps()
{
	fbo_blur_vertical->bind();
	{
		glClearColor(0.f,0.f,0.f,1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		shader_loader->push_bind(shader_blur_vertical);
		fbo_extraction->bind_rt(shader_blur_vertical->getFS(),0);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		quad->render(0);
		fbo_extraction->unbind_rt(0);
		shader_loader->pop_bind();
	} fbo_blur_vertical->unbind();

	fbo_blur_horizontal->bind();
	{
		glClearColor(0.f,0.f,0.f,1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		shader_loader->push_bind(shader_blur_horizontal);
		fbo_blur_vertical->bind_rt(shader_blur_horizontal->getFS(),0);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
		quad->render(0);
		fbo_blur_vertical->unbind_rt(0);
		shader_loader->pop_bind();
	} fbo_blur_horizontal->unbind();
}

void Bloom_Pass::render_final_step()
{
	shader_loader->push_bind(shader_final);
	fbo_blur_horizontal->bind_rt(shader_final->getFS(),0);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
	quad->render(0);
	fbo_blur_horizontal->unbind_rt(0);
	shader_loader->pop_bind();
}


void Bloom_Pass::end()
{
}

void Bloom_Pass::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
