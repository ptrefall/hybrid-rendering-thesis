#include "Raytrace_Pass.h"
#include "../../../Raytracer/OptixRender.h"

#include "../../../Scene/Camera.h"

using namespace Render;

Raytrace_Pass::Raytrace_Pass(const GBuffer_PassPtr &g_buffer_pass, unsigned int w, unsigned int h, const std::string &resource_dir)
	: g_buffer_pass(g_buffer_pass), w(w), h(h)
{
	raytracer = std::make_shared<Raytracer::OptixRender>(g_buffer_pass, w, h, resource_dir + "optix\\");
}

void Raytrace_Pass::begin()
{
}

void Raytrace_Pass::render()
{
	
}

void Raytrace_Pass::end()
{
}

void Raytrace_Pass::bind(unsigned int active_program)
{
}

void Raytrace_Pass::unbind()
{
}

void Raytrace_Pass::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
}
