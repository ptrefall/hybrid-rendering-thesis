#include "Tex2D.h"

using namespace Render;

Tex2D::Tex2D()
	: handle(0)
{
}

Tex2D::Tex2D(const T2DTexParams &tex_params)
	: internal_format(tex_params.internal_format), format(tex_params.format), type(tex_params.type), w(tex_params.w), h(tex_params.h), wrap_mode(tex_params.wrap_mode), data(tex_params.data)
{
	glGenTextures(1, &handle);
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	glGenerateMipmap(GL_TEXTURE_2D);
}

Tex2D::~Tex2D()
{
	glDeleteTextures(1, &handle);
}

void Tex2D::init(const T2DTexParams &tex_params)
{
	if(handle == 0)
		return;

	internal_format = tex_params.internal_format;
	format = tex_params.format;
	type = tex_params.type;
	w = tex_params.w;
	h = tex_params.h;
	wrap_mode = tex_params.wrap_mode;
	data = tex_params.data;

	glGenTextures(1, &handle);
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Tex2D::update(const T2DTexParams &tex_params)
{
	internal_format = tex_params.internal_format;
	format = tex_params.format;
	type = tex_params.type;
	w = tex_params.w;
	h = tex_params.h;
	wrap_mode = tex_params.wrap_mode;
	data = tex_params.data;

	if(handle == 0)
	{
		glGenTextures(1, &handle);
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, nullptr);
	}

	bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Tex2D::reset()
{
	glDeleteTextures(1, &handle);
	handle = 0;
}

void Tex2D::bind()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Tex2D::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}