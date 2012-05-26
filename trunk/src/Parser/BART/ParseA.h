#pragma once

#include <stdio.h>

namespace File { namespace BART
{
	struct anim_def;
}}

namespace Parser { namespace BART
{
	class ParseA
	{
	public:
		static void parse(FILE* f, File::BART::anim_def &anim);

	private:
		static void parseAnimParams(FILE *fp, File::BART::anim_def &anim);
		static void setupAnimParams( float start, float end, int num_frames, File::BART::anim_def &anim );
	};
}}