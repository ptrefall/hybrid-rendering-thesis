#include "DeferredRender.h"

#include "GBuffer.h"

using namespace Render;

DeferredRender::DeferredRender(const GBufferPtr &g_buffer, unsigned int w, unsigned int h)
	: g_buffer(g_buffer), w(w), h(h)
{
}

void DeferredRender::render()
{
	//Bind orthographic projection
	//Bind shader program
	g_buffer->bind();
	//Draw quad
	g_buffer->unbind();
}

void DeferredRender::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
