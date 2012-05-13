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

		Render::Tex2DPtr load(const std::string &filename);

	private:
		std::string base_dir;
	};
}
