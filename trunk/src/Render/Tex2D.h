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
		unsigned int bpp;
		unsigned int w;
		unsigned int h;
		unsigned int wrap_mode;
		unsigned char *data;
		T2DTexParams(unsigned int internal_format, unsigned int format, unsigned int type, unsigned int bpp, unsigned int w, unsigned int h, unsigned int wrap_mode = GL_CLAMP_TO_EDGE, unsigned char *data = nullptr)
		: internal_format(internal_format), format(format), type(type), bpp(bpp), w(w), h(h), wrap_mode(wrap_mode), data(data)
		{}
	};

	class Tex2D
	{
	public:
		Tex2D();
		Tex2D(const T2DTexParams &tex_params);
		~Tex2D();
 
		void init(const T2DTexParams &tex_params);
		void update(const T2DTexParams &tex_params);
		void update(void *data, bool update_client_data = false);
		void update(void *data, unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, bool update_client_data = false);
		void reset();

		void bind();
		void unbind();

		unsigned int getHandle() const { return handle; }

		unsigned int getInternalFormat() const { return internal_format; }
		unsigned int getFormat() const { return format; }
		unsigned int getType() const { return type; }
		unsigned int getBpp() const { return bpp; }
		unsigned int getWidth() const { return w; }
		unsigned int getHeight() const { return h; }
		unsigned int getWrapMode() const { return wrap_mode; }
		unsigned char *getData() const { return data; }

		//unsigned char *downloadData();
		void download(void *data, bool to_client);

	private:
		unsigned int handle;
		unsigned int internal_format;
		unsigned int format;
		unsigned int type;
		unsigned int bpp;

		unsigned int w;
		unsigned int h;

		unsigned int wrap_mode;

		unsigned char *data;
	};
}
