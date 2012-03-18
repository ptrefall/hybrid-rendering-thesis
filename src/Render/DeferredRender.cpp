#include "DeferredRender.h"

using namespace Render;

DeferredRender::DeferredRender(const GBufferPtr &g_buffer)
	: g_buffer(g_buffer), w(800), h(600)
{
}

void DeferredRender::render()
{
}
