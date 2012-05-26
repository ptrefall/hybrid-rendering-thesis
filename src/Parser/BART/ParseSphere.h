#pragma once

#include <stdio.h>
#include <vector>
#include <memory>

namespace Render
{
	class Material; typedef std::shared_ptr<Material> MaterialPtr;
}

namespace File { namespace BART
{
	struct sphere_t;
}}

namespace Parser { namespace BART
{
	class ParseSphere
	{
	public:
		static void parse(FILE* f, const Render::MaterialPtr &material, std::vector<File::BART::sphere_t> &spheres);
	};
}}