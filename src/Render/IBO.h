#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class IBO;
	typedef std::shared_ptr<IBO> IBOPtr;
	
	class IBO
	{
	public:
		IBO(const std::vector<unsigned int> &indices, const unsigned int &draw_type);
		~IBO();

		void bind();
		void unbind();
		unsigned int getHandle() const { return handle; }

		unsigned int size() const { return index_size; }

	private:
		unsigned int handle;
		unsigned int index_size;
	};
}
