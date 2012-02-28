#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace GM { namespace Render
{
	class VAO;
	typedef std::shared_ptr<VAO> VAOPtr;
	
	class VAO
	{
	public:
		VAO();
		~VAO();

		void bind();
		void unbind();

	private:
		unsigned int handle;
	};
}}
