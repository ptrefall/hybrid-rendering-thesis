#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>
#include <string>

namespace Render
{
	class Tex2DArray;
	typedef std::shared_ptr<Tex2DArray> Tex2DArrayPtr;
	
	struct Tex2DArrayParams
	{
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;
		unsigned int w;
		unsigned int h;
		unsigned int slice_count;
		unsigned int wrap_mode;
		unsigned char *data;
		Tex2DArrayParams(unsigned int internal_format, unsigned int format, unsigned int type, unsigned int w, unsigned int h, unsigned int slice_count, unsigned int wrap_mode = GL_CLAMP_TO_EDGE, unsigned char *data = nullptr)
		: internal_format(internal_format), format(format), type(type), w(w), h(h), slice_count(slice_count), wrap_mode(wrap_mode), data(data)
		{}
	};

	class Tex2DArray
	{
	public:
		Tex2DArray(const Tex2DArrayParams &tex_params);
		~Tex2DArray();

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
		unsigned int slice_count;

		unsigned int wrap_mode;

		unsigned char *data;
	};
}
