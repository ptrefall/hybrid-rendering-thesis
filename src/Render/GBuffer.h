#pragma once

#include <memory>

namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class GBuffer
	{
	public:
		GBuffer();

		void begin();
		void end();

		void reshape(unsigned int w, unsigned int h) { this->w = w; this->h = h; }

	private:
		unsigned int w;
		unsigned int h;
	};
}
