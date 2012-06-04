#include "PBO.h"

using namespace Render;

PBO::PBO(const unsigned int &size, const unsigned int &draw_type, bool unpack)
	: unpack(unpack)
{
	glGenBuffers(1, &handle);
	bind();
	if(unpack)
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, draw_type);
	else
		glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, draw_type);
}

PBO::~PBO()
{
	glDeleteBuffers(1, &handle);
}

void PBO::bind()
{
	if(unpack)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, handle);
	else
		glBindBuffer(GL_PIXEL_PACK_BUFFER, handle);
}

void PBO::unbind()
{
	if(unpack)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	else
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}
