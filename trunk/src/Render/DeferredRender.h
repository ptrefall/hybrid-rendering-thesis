#pragma once

#include "../Scene/Quad.h"
#include "../File/ShaderLoader.h"
#include "Shader.h"

#include <memory>

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
		void render();

		void reshape(unsigned int w, unsigned int h);

	private:
		unsigned int w;
		unsigned int h;

		GBufferPtr g_buffer;
		File::ShaderLoaderPtr shader_loader;
		ShaderPtr shader;


		Scene::QuadPtr quad;
	};
}
