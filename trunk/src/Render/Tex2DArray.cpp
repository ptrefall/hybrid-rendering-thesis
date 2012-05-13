#include "Tex2DArray.h"

using namespace Render;

Tex2DArray::Tex2DArray(const Tex2DArrayParams &tex_params)
	: internal_format(tex_params.internal_format), format(tex_params.format), type(tex_params.type), w(tex_params.w), h(tex_params.h), slice_count(tex_params.slice_count), wrap_mode(tex_params.wrap_mode), data(tex_params.data)
{
	glGenTextures(1, &handle);
	bind();
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internal_format, w, h, slice_count, 0, format, type, data);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap_mode);
}

Tex2DArray::~Tex2DArray()
{
	glDeleteTextures(1, &handle);
}

void Tex2DArray::bind()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
}

void Tex2DArray::unbind()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
