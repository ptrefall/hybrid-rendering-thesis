#pragma once

#include <glm\glm.hpp>
#include <stdio.h>
#include <vector>
#include <memory>

namespace Render
{
	class Material; typedef std::shared_ptr<Material> MaterialPtr;
}

namespace Parser { namespace BART
{
	class ParseFill
	{
	public:
		static void parse(FILE* f, std::vector<Render::MaterialPtr> &materials);

	private:
		static void addMaterial( const glm::vec3& amb, const glm::vec3& dif, const glm::vec3& spc, float phong_pow, float transmittance, float ior, std::vector<Render::MaterialPtr> &materials );
	};
}}