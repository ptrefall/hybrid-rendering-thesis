#include "Tex2D.h"

using namespace Render;

Tex2D::Tex2D(const T2DTexParams &tex_params)
	: internal_format(tex_params.internal_format), format(tex_params.format), type(tex_params.type), w(tex_params.w), h(tex_params.h), data(tex_params.data)
{
	glGenTextures(1, &handle);
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Tex2D::~Tex2D()
{
	glDeleteTextures(1, &handle);
}

void Tex2D::bind()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Tex2D::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
