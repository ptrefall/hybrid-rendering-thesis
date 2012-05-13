#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>
#include <string>

namespace Render
{
	class Tex2D;
	typedef std::shared_ptr<Tex2D> Tex2DPtr;
	
	struct T2DTexParams
	{
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;
		unsigned int w;
		unsigned int h;
		unsigned char *data;
		T2DTexParams(unsigned int internal_format, unsigned int format, unsigned int type, unsigned int w, unsigned int h, unsigned char *data = nullptr)
		: internal_format(internal_format), format(format), type(type), w(w), h(h), data(data)
		{}
	};

	class Tex2D
	{
	public:
		Tex2D(const T2DTexParams &tex_params);
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

		unsigned char *data;
	};
}
