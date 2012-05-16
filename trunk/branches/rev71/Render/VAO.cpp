#include "VAO.h"
#include <GL3/gl3w.h>

using namespace Render;

VAO::VAO()
{
	glGenVertexArrays(1, &handle);
	bind();
}

VAO::~VAO()
{
	glDeleteVertexArrays(1, &handle);
}

void VAO::bind()
{
	glBindVertexArray(handle);
}

void VAO::unbind()
{
	glBindVertexArray(0);
}
