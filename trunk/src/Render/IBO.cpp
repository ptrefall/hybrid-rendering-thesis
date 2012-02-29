#include "IBO.h"

using namespace Render;

IBO::IBO(const std::vector<unsigned int> &indices, const unsigned int &draw_type)
{
	glGenBuffers(1, &handle);
	bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], draw_type);
}

IBO::~IBO()
{
	glDeleteBuffers(1, &handle);
}

void IBO::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
}

void IBO::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
