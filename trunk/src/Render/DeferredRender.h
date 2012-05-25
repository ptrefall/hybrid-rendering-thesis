#pragma once

#include "../Scene/Quad.h"
#include "../File/ShaderLoader.h"
#include "Tex2D.h"
#include "Material.h"
#include "Shader.h"
#include "Uniform.h"

#include <memory>
#include <vector>

namespace Render
{
	class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;

	class DeferredRender;
	typedef std::shared_ptr<DeferredRender> DeferredRenderPtr;

	class DeferredRender
	{
	public:
		DeferredRender(const GBufferPtr &g_buffer, const File::ShaderLoaderPtr &shader_loader, unsigned int w, unsigned int h);
		void begin();
		void render();
		void end();

		void reshape(unsigned int w, unsigned int h);

		ShaderPtr getShader() const { return shader; }
		void reloadShaders();

		MaterialPtr addMaterial(const MaterialPtr &material) { materials.push_back(material); return material; }
        void setRayTexture(const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform);
	private:
		unsigned int w;
		unsigned int h;

		GBufferPtr g_buffer;
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader;
		UniformPtr camPos;

		Scene::QuadPtr quad;

		std::vector<MaterialPtr> materials;

        Render::Tex2DPtr tex;
		Render::UniformPtr tex_uniform;
	};
}
