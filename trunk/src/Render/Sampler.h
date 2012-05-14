#pragma once

#include <GL3\gl3w.h>
#include <Eigen\Eigen>

#include <memory>
#include <string>

namespace Render
{
	class Sampler;
	typedef std::shared_ptr<Sampler> SamplerPtr;

	class Sampler
	{
	public:
		Sampler(int wrap_mode = GL_CLAMP_TO_EDGE);
		~Sampler();
		void bind(unsigned int unit = 0);
		void unbind(unsigned int unit = 0);

		void setParameteri(unsigned int parameter, int value);
		void setParameterf(unsigned int parameter, float value);
		void setParameterfv(unsigned int parameter, float *value);

	private:
		unsigned int handle;
	};
}