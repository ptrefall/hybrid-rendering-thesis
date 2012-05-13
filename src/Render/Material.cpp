#include "Material.h"

using namespace Render;

Material::Material(const MaterialParams &params)
	: id(params.id), ambient(params.ambient), diffuse(params.diffuse), specular(params.specular), pp_t_ior(params.phong_pow,params.transparency,params.index_of_refraction)
{
	u_id = std::make_shared<Uniform>("material_id");
	u_amb = std::make_shared<Uniform>("ambient_mat");
	u_diff = std::make_shared<Uniform>("diffuse_mat");
	u_spec = std::make_shared<Uniform>("specular_mat");
	u_pp_t_ior = std::make_shared<Uniform>("pp_t_ior_mat");
}

Material::~Material()
{
}

void Material::bind_id(unsigned int active_program)
{
	u_id->bind((float)id, active_program);
}

void Material::bind_data(unsigned int active_program)
{
	u_amb->bind(ambient, active_program);
	u_diff->bind(diffuse, active_program);
	u_spec->bind(specular, active_program);
	u_pp_t_ior->bind(pp_t_ior, active_program);
}
