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
	struct cone_t;
}}

namespace Parser { namespace BART
{
	class ParseCone
	{
	public:
		static void parse(FILE* f, const Render::MaterialPtr &material, std::vector<File::BART::cone_t> &cones);
	};
}}