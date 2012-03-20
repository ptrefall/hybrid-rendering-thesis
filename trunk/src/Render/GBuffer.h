#pragma once

#include "FBO.h"
#include "../File/ShaderLoader.h"
#include "Shader.h"
#include "Uniform.h"

#include <memory>

namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class GBuffer
	{
	public:
		GBuffer(const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);

		void begin();
		void end();

		void bind();
		void unbind();

		void reshape(unsigned int w, unsigned int h);

	private:
		unsigned int w;
		unsigned int h;

		unsigned int temp_w;
		unsigned int temp_h;

		FBOPtr fbo;
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader;

		UniformPtr mvp;
		UniformPtr mv;
		UniformPtr n_wri;
	};
}
