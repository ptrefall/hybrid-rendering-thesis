#include "Sampler.h"
#include <iostream>

using namespace Render;

Sampler::Sampler(int wrap_mode) 
	: handle(0)
{
	glGenSamplers(1, &handle);

	setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	setParameteri(GL_TEXTURE_WRAP_S, wrap_mode);
	setParameteri(GL_TEXTURE_WRAP_T, wrap_mode);
	setParameteri(GL_TEXTURE_WRAP_R, wrap_mode);
	float border_color[4] = {0,0,0,0};
	setParameterfv(GL_TEXTURE_BORDER_COLOR, border_color);
	setParameterf(GL_TEXTURE_MIN_LOD, -1000.f);
	setParameterf(GL_TEXTURE_MAX_LOD, 1000.f);
	setParameterf(GL_TEXTURE_LOD_BIAS, 0.0f);
	setParameteri(GL_TEXTURE_COMPARE_MODE, GL_NONE);
	setParameteri(GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

Sampler::~Sampler()
{
	glDeleteSamplers(1, &handle);
}

void Sampler::bind(unsigned int unit)
{
	glBindSampler(unit, handle);
}

void Sampler::unbind(unsigned int unit)
{
	glBindSampler(unit, 0);
}

void Sampler::setParameteri(unsigned int parameter, int value)
{
	glSamplerParameteri(handle, parameter, value);
}

void Sampler::setParameterf(unsigned int parameter, float value)
{
	glSamplerParameterf(handle, parameter, value);
}

void Sampler::setParameterfv(unsigned int parameter, float *value)
{
	glSamplerParameterfv(handle, parameter, value);
}
