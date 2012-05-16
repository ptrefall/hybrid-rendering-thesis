#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class RT;
	typedef std::shared_ptr<RT> RTPtr;
	
	class RT
	{
	public:
		RT(unsigned int type, unsigned int w, unsigned int h);
		~RT();

		void bind();
		void unbind();

		unsigned int getHandle() const { return handle; }

	private:
		unsigned int handle;
		unsigned int type;

		unsigned int w;
		unsigned int h;
	};
}
