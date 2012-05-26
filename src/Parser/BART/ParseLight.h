#pragma once

#include <glm\glm.hpp>
#include <string>
#include <vector>

namespace File { namespace BART
{
	struct light_t;
}}

namespace Parser { namespace BART
{
	class ParseLight
	{
	public:
		static void parse(FILE* f, std::vector<File::BART::light_t> &lights);

	private:
		static void addLight( const std::string name, const glm::vec3& pos, const glm::vec3& col, std::vector<File::BART::light_t> &lights );
	};
}}