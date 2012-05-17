#pragma once

#include "../Render/Tex2D.h"
#include "../Render/Tex2DArray.h"

#include <memory>
#include <string>

namespace File
{
	class TextureLoader;
	typedef std::shared_ptr<TextureLoader> TextureLoaderPtr;

	class TextureLoader
	{
	public:
		TextureLoader(const std::string &base_dir);

		Render::Tex2DPtr load(const std::string &filename, unsigned int wrap_mode = GL_CLAMP_TO_EDGE);
		Render::Tex2DArrayPtr load_array(const std::string &filename, unsigned int width, unsigned int height, unsigned int slice_count_width, unsigned int slice_count_height, unsigned int wrap_mode = GL_CLAMP_TO_EDGE);

		//void save(const Render::Tex2DPtr &tex, const std::string &location);

	private:
		struct internal_tex_data
		{
			int bpp;
			int w;
			int h;
			int format;
			int type;
			unsigned char *data;
			internal_tex_data() : bpp(0), w(0), h(0), format(0), data(nullptr) {}
		};
		internal_tex_data internal_load(unsigned int &il_handle, const std::string &filename);
		std::string base_dir;
	};
}
