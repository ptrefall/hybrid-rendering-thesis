#pragma once

#include <memory>
#include <string>

namespace Render
{
	class ShaderConstants
	{
	public:
		//These are equivalent to the defines in shader
		static unsigned int Diffuse()	{ return 0; }
		static unsigned int Position()	{ return 1; }
		static unsigned int Normal()	{ return 2; }
		static unsigned int TexCoord()	{ return 3; }
		static unsigned int Tangent()	{ return 4; }
		static unsigned int Bitangent()	{ return 5; }
		static unsigned int FragColor()	{ return 0; }
	};
}