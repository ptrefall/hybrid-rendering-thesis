#pragma once

#include <glm\glm.hpp>
#include <stdio.h>
#include <string>

namespace File { namespace BART
{
	struct active_def;
}}

namespace Parser { namespace BART
{
	class ParseTextureStuff
	{
	public:
		static void parse(FILE* f, const std::string &base_dir, const std::string &sceneFolder, File::BART::active_def &active);

	private:
		static void parseTexturedTriangle(FILE *f, std::string base_dir, std::string sceneFolder, File::BART::active_def &active);
		static void parseAnimatedTriangle(FILE *f);

		static void addTexturedTrianglePatch( const std::string& texturename, glm::vec3* verts, glm::vec3* norms, glm::vec2* uv, File::BART::active_def &active );
		static void addTexturedTriangle( const std::string& texturename, glm::vec3* verts, glm::vec2* uv, File::BART::active_def &active );
	};
}}