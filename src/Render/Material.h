#pragma once

#include <GL3/gl3w.h>
#include "Uniform.h"

#include <Eigen\Eigen>

#include <memory>
#include <vector>
#include <string>

namespace Render
{
	class Material;
	typedef std::shared_ptr<Material> MaterialPtr;
	
	struct MaterialParams
	{
		unsigned int id;
		Eigen::Vector3f ambient, diffuse, specular;
		float phong_pow;
		float transparency;
		float index_of_refraction;
		MaterialParams() {}
		MaterialParams(unsigned int id, Eigen::Vector3f ambient, Eigen::Vector3f diffuse, Eigen::Vector3f specular, float phong_pow, float transparency, float index_of_refraction)
			: id(id), ambient(ambient), diffuse(diffuse), specular(specular), phong_pow(phong_pow), transparency(transparency), index_of_refraction(index_of_refraction)
		{}
	};

	class Material
	{
	public:
		Material(const MaterialParams &params);
		~Material();

		void bind_id(unsigned int active_program);
		void bind_data(unsigned int active_program);

	private:
		unsigned int id;
		Eigen::Vector3f ambient, diffuse, specular, pp_t_ior;

		UniformPtr u_id, u_amb, u_diff, u_spec, u_pp_t_ior;
	};
}
