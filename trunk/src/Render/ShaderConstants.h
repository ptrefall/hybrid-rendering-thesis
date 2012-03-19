#pragma once

#include <memory>
#include <string>

namespace Render
{
	class ShaderConstants
	{
	public:
		static unsigned int Diffuse()	{ return 0; }
		static unsigned int Position()	{ return 1; }
		static unsigned int Normal()	{ return 2; }
		static unsigned int TexCoord()	{ return 3; }
	};
}