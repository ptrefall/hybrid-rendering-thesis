#pragma once

#include "../../../Scene/Quad.h"
#include "../../../File/ShaderLoader.h"
#include "../../FBO.h"
#include "../../Tex2D.h"
#include "../../Material.h"
#include "../../Shader.h"
#include "../../Uniform.h"

#include <memory>
#include <vector>

namespace Render
{
	class Final_Pass;
	typedef std::shared_ptr<Final_Pass> Final_PassPtr;

	class Bloom_Pass;
	typedef std::shared_ptr<Bloom_Pass> Bloom_PassPtr;

	class Bloom_Pass
	{
	public:
		Bloom_Pass(const Final_PassPtr &final_pass, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);
		void begin();
		void render_extraction_step();
		void render_blur_steps();
		void render_final_step();
		void end();

		void reshape(unsigned int w, unsigned int h);
	private:
		unsigned int w;
		unsigned int h;

		Final_PassPtr final_pass;

		FBOPtr fbo_extraction, fbo_blur_vertical, fbo_blur_horizontal;
		unsigned int temp_w;
		unsigned int temp_h;
		
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader_extraction, shader_blur_vertical, shader_blur_horizontal, shader_final;

		Scene::QuadPtr quad;
	};
}
