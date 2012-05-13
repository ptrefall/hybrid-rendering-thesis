#pragma once

#include "../Render/Tex2D.h"

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

	private:
		std::string base_dir;
	};
}
