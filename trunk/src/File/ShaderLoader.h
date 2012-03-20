#pragma once

#include "../Render/Shader.h"

#include <memory>
#include <string>

namespace File
{
	class ShaderLoader;
	typedef std::shared_ptr<ShaderLoader> ShaderLoaderPtr;

	class ShaderLoader
	{
	public:
		ShaderLoader(const std::string &base_dir);

		Render::ShaderPtr load(const std::string &vs_filename, const std::string &gs_filename = std::string(), const std::string &fs_filename = std::string());

	private:
		std::string loadContents(const std::string &filename);
		std::string base_dir;
	};
}
