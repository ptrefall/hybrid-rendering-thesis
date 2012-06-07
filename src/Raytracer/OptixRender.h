#pragma once

#include "../Render/Tex2D.h"
#include "../Render/PBO.h"
#include "../Scene/Quad.h"
#include "../File/ShaderLoader.h"
#include <Optix/optixu/optixpp_namespace.h>

#include <memory>

namespace Render
{
    class GBuffer_Pass; typedef std::shared_ptr<GBuffer_Pass> GBuffer_PassPtr;
}

namespace Raytracer
{
	class OptixRender;
	typedef std::shared_ptr<OptixRender> OptixRenderRenderPtr;

	class OptixRender
	{
	public:
		OptixRender(const Render::GBuffer_PassPtr &g_buffer_pass, unsigned int width, unsigned int height, const std::string& baseDir);
		void render();
		void reshape(unsigned int width, unsigned int height);

	private:
        Render::GBuffer_PassPtr g_buffer_pass;
		unsigned int width;
		unsigned int height;
        optix::Context context; 
        std::string baseDir;

    private:
		optix::Context minimalCreateContext();
		void createGBuffers();
		void pbo2Texture();
		unsigned int getBufferAlignment(optix::Buffer buffer);

		optix::Buffer g_buffer_diffuse;
		optix::Buffer g_buffer_position;
		optix::Buffer g_buffer_normal;
		
		Render::PBOPtr g_buffer_diffuse_pbo;
		Render::PBOPtr g_buffer_position_pbo;
		Render::PBOPtr g_buffer_normal_pbo;
	};
}
