#pragma once

#include <memory>
#include <string>

namespace Render
{
	class Tex2D; typedef std::shared_ptr<Tex2D> Tex2DPtr;
}

namespace File
{
	class AssetManager; typedef std::shared_ptr<AssetManager> AssetManagerPtr;
	class TextureLoader; typedef std::shared_ptr<TextureLoader> TextureLoaderPtr;
	

	class AssetManager
	{
	public:
		~AssetManager();
		AssetManager(const std::string &base_dir);
		Render::Tex2DPtr getTex2DRelativePath(const std::string &filename, bool isRepeated );// = false);
		Render::Tex2DPtr getTex2DAbsolutePath(const std::string &filename, bool isRepeated );//= false);
	

	private:
		std::string base_dir;
		File::TextureLoaderPtr tex_loader;
	};
}
