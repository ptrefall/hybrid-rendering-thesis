#include "RT.h"

using namespace Render;

RT::RT(unsigned int type, unsigned int w, unsigned int h)
	: type(type), w(w), h(h)
{
	glGenRenderbuffers(1, &handle);
	bind();
	glRenderbufferStorage(GL_RENDERBUFFER, type, w, h);
}

RT::~RT()
{
	glDeleteRenderbuffers(1, &handle);
}

void RT::bind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, handle);
}

void RT::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
