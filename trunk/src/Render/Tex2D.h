#pragma once

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class Tex2D;
	typedef std::shared_ptr<Tex2D> Tex2DPtr;
	
	struct T2DShaderParams
	{
		unsigned int handle;
		unsigned int position;
		T2DShaderParams(unsigned int handle, unsigned int position) : handle(handle), position(position) {}
	};

	struct T2DTexParams
	{
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;
		unsigned int w;
		unsigned int h;
		T2DTexParams(unsigned int internal_format, unsigned int format, unsigned int type, unsigned int w, unsigned int h)
		: internal_format(internal_format), format(format), type(type), w(w), h(h)
		{}
	};

	class Tex2D
	{
	public:
		Tex2D(const T2DShaderParams &shader_params, const T2DTexParams &tex_params);
		~Tex2D();

		void bind(int slot);
		void unbind();

		unsigned int getHandle() const { return handle; }

	private:
		unsigned int handle;
		unsigned int shader_handle;
		unsigned int shader_position;
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;

		unsigned int w;
		unsigned int h;
	};
}
