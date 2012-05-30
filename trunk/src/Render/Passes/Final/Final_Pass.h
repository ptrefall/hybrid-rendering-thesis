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
	class GBuffer_Pass;
	typedef std::shared_ptr<GBuffer_Pass> GBuffer_PassPtr;

	class Final_Pass;
	typedef std::shared_ptr<Final_Pass> Final_PassPtr;

	class Final_Pass
	{
	public:
		Final_Pass(const GBuffer_PassPtr &g_buffer_pass, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);
		
		void begin();
		void render();
		void end();

		void bind(unsigned int active_program);
		void unbind();

		void reshape(unsigned int w, unsigned int h);

		ShaderPtr getShader() const { return shader; }
		void reloadShaders();

		MaterialPtr addMaterial(const MaterialPtr &material) { materials.push_back(material); return material; }
        void setRayTexture(const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform);
	private:
		unsigned int w;
		unsigned int h;

		GBuffer_PassPtr g_buffer_pass;
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader;
		UniformPtr camPos;

		FBOPtr fbo;
		unsigned int temp_w;
		unsigned int temp_h;

		Scene::QuadPtr quad;

		std::vector<MaterialPtr> materials;

        Render::Tex2DPtr tex;
		Render::UniformPtr tex_uniform;
	};
}
