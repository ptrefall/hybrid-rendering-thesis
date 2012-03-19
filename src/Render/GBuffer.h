#pragma once

#include "FBO.h"

#include <memory>

namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class GBuffer
	{
	public:
		GBuffer(unsigned int w, unsigned int h);

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
	};
}
