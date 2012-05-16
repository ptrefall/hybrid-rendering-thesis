#pragma once

#include "../Render/Material.h"

#include <memory>
#include <string>

namespace File
{
	class MaterialLoader;
	typedef std::shared_ptr<MaterialLoader> MaterialLoaderPtr;

	class MaterialLoader
	{
	public:
		MaterialLoader(const std::string &base_dir);

		Render::MaterialPtr load(const std::string &filename);

	private:
		std::string base_dir;
		unsigned int material_counter;
	};
}
