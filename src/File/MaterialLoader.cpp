#include "MaterialLoader.h"

#include "..\Parser\INIParser.h"

#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

using namespace File;

MaterialLoader::MaterialLoader(const std::string &base_dir)
	: base_dir(base_dir)
{
}

Render::MaterialPtr MaterialLoader::load(const std::string &filename)
{
	Render::MaterialParams params;
	
	ini::Parser parser(base_dir + filename);

	params.ambient[0] = parser.getFloat("Material", "ambient_r", 0.0f);
	params.ambient[1] = parser.getFloat("Material", "ambient_g", 0.0f);
	params.ambient[2] = parser.getFloat("Material", "ambient_b", 0.0f);

	params.diffuse[0] = parser.getFloat("Material", "diffuse_r", 0.0f);
	params.diffuse[1] = parser.getFloat("Material", "diffuse_g", 0.0f);
	params.diffuse[2] = parser.getFloat("Material", "diffuse_b", 0.0f);

	params.specular[0] = parser.getFloat("Material", "specular_r", 0.0f);
	params.specular[1] = parser.getFloat("Material", "specular_g", 0.0f);
	params.specular[2] = parser.getFloat("Material", "specular_b", 0.0f);
	
	params.phong_pow = parser.getFloat("Material", "phong_pow", 0.0f);
	params.transparency = parser.getFloat("Material", "transparency", 0.0f);
	params.index_of_refraction = parser.getFloat("Material", "index_of_refraction", 0.0f);

	auto material = std::make_shared<Render::Material>(params);
	return material;
}
