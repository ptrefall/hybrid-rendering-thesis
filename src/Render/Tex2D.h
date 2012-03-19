#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class Tex2D;
	typedef std::shared_ptr<Tex2D> Tex2DPtr;
	
	class Tex2D
	{
	public:
		Tex2D(unsigned int internal_format, unsigned int format, unsigned int type, unsigned int w, unsigned int h);
		~Tex2D();

		void bind();
		void unbind();

		unsigned int getHandle() const { return handle; }

	private:
		unsigned int handle;
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;

		unsigned int w;
		unsigned int h;
	};
}
