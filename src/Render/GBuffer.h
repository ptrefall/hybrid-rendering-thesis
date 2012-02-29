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

	private:
		unsigned int screen_width;
		unsigned int screen_height;
	};
}
