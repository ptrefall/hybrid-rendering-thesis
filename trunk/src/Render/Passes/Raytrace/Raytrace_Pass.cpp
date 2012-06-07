#include "Raytrace_Pass.h"
#include "../../../Raytracer/OptixRender.h"

#include "../../../Scene/Camera.h"

using namespace Render;

Raytrace_Pass::Raytrace_Pass(const GBuffer_PassPtr &g_buffer_pass, unsigned int w, unsigned int h, const std::string &resource_dir)
	: g_buffer_pass(g_buffer_pass), w(w), h(h)
{
	raytracer = std::make_shared<Raytracer::OptixRender>(g_buffer_pass, w, h, resource_dir + "optix\\");
	raytrace_tex_uniform = std::make_shared<Uniform>("TEX_RAY");
}

void Raytrace_Pass::begin()
{
	glFlush();
}

void Raytrace_Pass::render()
{
	raytracer->render();
}

void Raytrace_Pass::end()
{
}

void Raytrace_Pass::bind(unsigned int active_program, unsigned int index_offset)
{
}

void Raytrace_Pass::unbind(unsigned int index_offset)
{
}

void Raytrace_Pass::reshape(unsigned int w, unsigned int h) 
{ 
	this->w = w;
	this->h = h; 
	raytracer->reshape(w,h);
}
