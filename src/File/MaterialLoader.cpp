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
	: base_dir(base_dir), material_counter(0)
{
}

Render::MaterialPtr MaterialLoader::load(const std::string &filename)
{
	Render::MaterialParams params;
	params.id = material_counter++;
	
	ini::Parser parser(base_dir + filename);

	params.ambient = parser.getVec3("Material", "ambient", glm::vec3(0.f) );
	params.diffuse = parser.getVec3("Material", "diffuse", glm::vec3(0.f) );
	params.specular = parser.getVec3("Material", "specular", glm::vec3(0.f) );

	params.phong_pow = parser.getFloat("Material", "phong_pow", 0.0f);
	params.transparency = parser.getFloat("Material", "transparency", 0.0f);
	params.index_of_refraction = parser.getFloat("Material", "index_of_refraction", 0.0f);

	auto material = std::make_shared<Render::Material>(params);
	return material;
}
