#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class PBO;
	typedef std::shared_ptr<PBO> PBOPtr;
	
	class PBO
	{
	public:
		PBO(const unsigned int &size, const unsigned int &draw_type, bool unpack = true);
		~PBO();

		void bind();
		void unbind();

		unsigned int getHandle() const { return handle; }

	private:
		unsigned int handle;
		bool unpack;
	};
}
