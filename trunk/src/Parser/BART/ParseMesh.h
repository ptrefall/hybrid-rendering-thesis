#pragma once

#include <glm\glm.hpp>
#include <vector>
#include <stdio.h>

namespace File { namespace BART
{
	struct active_def;
}}

namespace Parser { namespace BART
{
	class ParseMesh
	{
	public:
		static void parse(FILE* f, const std::string &base_dir, const std::string &sceneFolder, File::BART::active_def &active);

	private:
		static void getVectors(FILE *fp,char *type, std::vector<glm::vec3>& vecs);
		static void getTextureCoords(FILE *fp,char *texturename,std::vector<glm::vec2>& txts);
		static void getTriangles(FILE *fp,int *num_tris,std::vector<unsigned int>& indices, bool hasNorms, bool hasTexCoords);

		static void addMesh(	const std::vector<glm::vec3> &vertCoords, const std::vector<glm::vec3> &vertNormals, 
								const std::vector<glm::vec2> &texCoords, const std::vector<unsigned int> &indices, 
								File::BART::active_def &active );
	};
}}