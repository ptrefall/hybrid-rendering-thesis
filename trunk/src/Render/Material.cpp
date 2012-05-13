#include "Material.h"

using namespace Render;

Material::Material(const MaterialParams &params)
	: ambient(params.ambient), diffuse(params.diffuse), specular(params.specular), phong_pow(params.phong_pow), transparency(params.transparency), index_of_refraction(params.index_of_refraction)
{
}

Material::~Material()
{
}

void Material::bind()
{
}

void Material::unbind()
{
}
