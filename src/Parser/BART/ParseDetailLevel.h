#pragma once

#include <stdio.h>

namespace Parser { namespace BART
{
	class ParseDetailLevel
	{
	public:
		static void parse(FILE* f, unsigned int &detailLevel);
	};
}}