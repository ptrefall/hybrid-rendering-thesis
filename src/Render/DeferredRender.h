#pragma once

#include <memory>

namespace Render
{
	class DeferredRender;
	typedef std::shared_ptr<DeferredRender> DeferredRenderPtr;

	class DeferredRender
	{
	public:
		DeferredRender();
		void render();

	private:
		unsigned int screen_width;
		unsigned int screen_height;
	};
}
