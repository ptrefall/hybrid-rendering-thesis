#pragma once

#include <GL3/gl3w.h>
#include <vector>

#ifndef BUFFER_OFFSET
        /// BUFFER_OFFSET is used with some OpenGL functions to offset indexing correctly into the buffer.
        #define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + bytes)
#endif

namespace Render
{
	class ATTRIB
	{
	public:
		static void bind(	const unsigned int &shader,
							const std::string &name,
							const unsigned int &size_per_index,
							const unsigned int &data_type,
							bool normalized,
							const unsigned int &stride,
							const unsigned int &offset,
							const unsigned int &divisor = 0)
		{
			unsigned int loc = glGetAttribLocation(shader, name.c_str());
			bind(loc, size_per_index, data_type, normalized, stride, offset, divisor);
		}

		static void bind(	const unsigned int &loc,
							const unsigned int &size_per_index,
							const unsigned int &data_type,
							bool normalized,
							const unsigned int &stride,
							const unsigned int &offset,
							const unsigned int &divisor = 0)
		{
			if(loc < 0)
				throw std::runtime_error("Couldn't find location of attribute in shader");

			glVertexAttribPointer(loc, size_per_index, data_type, normalized, stride, BUFFER_OFFSET(offset));
			glEnableVertexAttribArray(loc);
			if(divisor > 0)
				glVertexAttribDivisor(loc, divisor);
		}
	};
}