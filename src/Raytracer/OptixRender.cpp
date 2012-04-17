#include "OptixRender.h"

using namespace Raytracer;

OptixRender::OptixRender(unsigned int w, unsigned int h)
	: w(w), h(h)
{
}

void OptixRender::render()
{
}

void OptixRender::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
