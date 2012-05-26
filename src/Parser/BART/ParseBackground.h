#pragma once

#include <glm\glm.hpp>
#include <stdio.h>

namespace Parser { namespace BART
{
	class ParseBackground
	{
	public:
		static void parse(FILE* f, glm::vec3 &bgcolor);
	};
}}