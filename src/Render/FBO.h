#pragma once

#include <GL3/gl3w.h>

#include "RT.h"
#include "Tex2D.h"
#include "Shader.h"
#include "Uniform.h"

#include <memory>
#include <vector>

namespace Render
{
	class FBO;
	typedef std::shared_ptr<FBO> FBOPtr;
	
	class FBO
	{
	public:
		FBO(unsigned int w, unsigned int h);
		~FBO();

		void bind();
		void unbind();

		void bind_rt(unsigned int active_program, unsigned int index_offset);
		void unbind_rt(unsigned int index_offset);

		void add(unsigned int attachment, const RTPtr &render_target);
		void add(unsigned int attachment, unsigned int texture_type, const std::string &sampler_name, const Tex2DPtr &render_texture);

		void check();

	private:
		unsigned int handle;

		unsigned int w;
		unsigned int h;

		std::vector<RTPtr> render_targets;
		std::vector<Tex2DPtr> render_textures;
		std::vector<UniformPtr> render_samplers;
	};
}
