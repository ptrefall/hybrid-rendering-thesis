#pragma once

#include <stdio.h>

namespace File { namespace BART
{
	struct camera_def;
}}

namespace Parser { namespace BART
{
	class ParseViewpoint
	{
	public:
		static void parse(FILE* f, File::BART::camera_def &cam);
	};
}}