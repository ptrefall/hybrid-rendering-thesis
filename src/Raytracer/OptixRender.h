#pragma once

#include "../Scene/Quad.h"
#include "../File/ShaderLoader.h"

#include <memory>

namespace Raytracer
{
	class OptixRender;
	typedef std::shared_ptr<OptixRender> OptixRenderRenderPtr;

	class OptixRender
	{
	public:
		OptixRender(unsigned int w, unsigned int h);
		void render();

		void reshape(unsigned int w, unsigned int h);

	private:
		unsigned int w;
		unsigned int h;
	};
}
