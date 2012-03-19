#pragma once

#include "../Render/Shader.h"

#include <string>

namespace File
{
	class ShaderLoader
	{
	public:
		ShaderLoader(const std::string &base_dir);

		Render::ShaderPtr load(const std::string &filename);

	private:
		std::string loadContents(const std::string &filename);
		std::string base_dir;
	};
}
