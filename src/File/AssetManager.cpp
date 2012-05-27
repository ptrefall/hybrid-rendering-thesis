#include "AssetManager.h"

#include "TextureLoader.h"

using namespace File;

AssetManager::AssetManager(const std::string &base_dir)
	: base_dir(base_dir)
{
	tex_loader = std::make_shared<File::TextureLoader>(base_dir+"textures\\");
}

Render::Tex2DPtr AssetManager::getTex2DRelativePath(const std::string &filename, bool isRepeated)
{
	return tex_loader->loadRelativePath(filename, isRepeated);
}

Render::Tex2DPtr AssetManager::getTex2DAbsolutePath(const std::string &filename, bool isRepeated)
{
	return tex_loader->loadAbsolutePath(filename, isRepeated);
}

AssetManager::~AssetManager()
{
	long tex_loader_count = tex_loader.use_count();
	tex_loader.reset();
}