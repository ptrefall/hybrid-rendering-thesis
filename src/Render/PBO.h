#pragma once

#include "Tex2D.h"

#include <GL3/gl3w.h>

#include <memory>
#include <vector>

namespace Render
{
	class PBO;
	typedef std::shared_ptr<PBO> PBOPtr;
	
	class PBO
	{
	public:
		PBO(const unsigned int &size, const unsigned int &draw_type, bool unpack = true);
		~PBO();

		void bind(bool unpack = true);
		void unbind(bool unpack = true);
		void align(unsigned int bits, bool unpack = true);

		unsigned int copyToTextureOnGPU(const Tex2DPtr &tex, unsigned int offset);
		unsigned int copyToTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access);
		unsigned int bufferFromTextureOnGPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type);
		unsigned int bufferFromTextureOnCPU(const Tex2DPtr &tex, unsigned int offset, unsigned int draw_type, unsigned int access);

		unsigned int getHandle() const { return handle; }

	private:
		unsigned int handle;
		bool bound;
	};
}
