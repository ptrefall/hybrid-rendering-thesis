#include "DeferredRender.h"

#include "GBuffer.h"
#include "../Scene/proto_camera.h"

using namespace Render;

DeferredRender::DeferredRender(const GBufferPtr &g_buffer, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h)
	: g_buffer(g_buffer), shader_loader(shader_loader), w(w), h(h)
{
	quad = std::make_shared<Scene::Quad>(w,h);
	shader = shader_loader->load("deferredRendering.vs", std::string(), "deferredRendering.fs");
	camPos = std::make_shared<Uniform>(shader->getFS(), "CamPos");
}

void DeferredRender::begin()
{
	shader_loader->push_bind(shader);
	g_buffer->bind(shader->getFS());

	camPos->bind(Scene::FirstPersonCamera::getSingleton()->getPos());

	if(materials.empty() == false)
	{
		for(auto it=materials.begin(); it!=materials.end(); ++it) //for(auto &material : materials)
			(*it)->bind_data(shader->getFS());
	}

    if(tex)
	{
		glActiveTexture(GL_TEXTURE0 + 0);
		tex->bind();
		if(tex_uniform)
			tex_uniform->bind(0);
	}
}

void DeferredRender::render()
{
	quad->render(0);
}

void DeferredRender::end()
{
	g_buffer->unbind();

    if(tex)
		tex->unbind();

	shader_loader->pop_bind();
}

void DeferredRender::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}

void DeferredRender::reloadShaders()
{
	puts("reload");
	shader = shader_loader->load("deferredRendering.vs", std::string(), "deferredRendering.fs");
}

void DeferredRender::setRayTexture(const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform)
{
    this->tex = tex;
    this->tex_uniform = uniform;
}
