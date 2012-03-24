#include "DeferredRender.h"

#include "GBuffer.h"

using namespace Render;

DeferredRender::DeferredRender(const GBufferPtr &g_buffer, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h)
	: g_buffer(g_buffer), shader_loader(shader_loader), w(w), h(h)
{
	quad = std::make_shared<Scene::Quad>(w,h);
	shader = shader_loader->load("deferredRendering.vs", std::string(), "deferredRendering.fs");
	camPos = std::make_shared<Uniform>(shader->getFS(), "CamPos");
}

void DeferredRender::render()
{
	shader->bind();
	g_buffer->bind(shader->getFS());
	quad->render();
	g_buffer->unbind();
}

void DeferredRender::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
