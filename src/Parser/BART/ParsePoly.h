#pragma once

#include <glm\glm.hpp>

#include <stdio.h>
#include <vector>
#include <memory>

namespace Render
{
	class Material; typedef std::shared_ptr<Material> MaterialPtr;
}

namespace File { namespace BART
{
	struct poly_t;
}}

namespace Parser { namespace BART
{
	class ParsePoly
	{
	public:
		static void parse(FILE* f);

		static void addPoly( const std::vector<glm::vec3> &vertCoords, const std::vector<glm::vec3> &vertNormals, const std::vector<glm::vec2> &texCoords );
	private:
		static void calcNormals( const std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals );
		static void calcNormals( const std::vector<glm::vec3> &vertices, const std::vector<unsigned int> &indices, std::vector<glm::vec3> &normals );
	};
}}