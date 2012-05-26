#pragma once

#include <glm\glm.hpp>
#include <stdio.h>
#include <functional>
#include <string>

namespace File{ namespace BART
{
	struct active_def;
}}

namespace Parser { namespace BART
{
	class ParseXform
	{
	public:
		static void parse(FILE* f, File::BART::active_def &active, const std::function<void(const std::string&, const glm::mat4&)> &pushNodeFunc);
		static void end(File::BART::active_def &active, const std::function<void()> &popNodeFunc);
	};
}}