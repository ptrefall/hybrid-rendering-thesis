#pragma once

#include <stdio.h>

struct AnimationList;

namespace Parser { namespace BART
{
	class ParseKeyFrames
	{
	public:
		static void parse(FILE* f, AnimationList *&mAnimations);
	};
}}