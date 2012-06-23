#pragma once

#include "../../Uniform.h"

#include <memory>
#include <vector>

namespace Raytracer
{
	class OptixRender; typedef std::shared_ptr<OptixRender> OptixRenderPtr;
}

namespace Render
{
	class GBuffer_Pass;
	typedef std::shared_ptr<GBuffer_Pass> GBuffer_PassPtr;

	class Raytrace_Pass;
	typedef std::shared_ptr<Raytrace_Pass> Raytrace_PassPtr;

	class Raytrace_Pass
	{
	public:
		Raytrace_Pass(const GBuffer_PassPtr &g_buffer_pass, unsigned int w, unsigned int h, const std::string &resource_dir);
		
		void begin();
		void render();
		void end();

		void bind(unsigned int active_program, unsigned int index_offset);
		void unbind(unsigned int index_offset);

		void reshape(unsigned int w, unsigned int h);
		Raytracer::OptixRenderPtr &getRaytracer() { return raytracer; }
	private:
		GBuffer_PassPtr g_buffer_pass;
		Raytracer::OptixRenderPtr raytracer;

		unsigned int w;
		unsigned int h;

		Render::UniformPtr raytrace_tex_uniform;
	};
}
