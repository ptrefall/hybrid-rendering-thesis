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

		Render::Tex2DPtr getRenderTexture() const { return diffuse_tex; }

	private:
        Render::GBuffer_PassPtr g_buffer_pass;
		unsigned int width;
		unsigned int height;
        optix::Context  context; 
		optix::Buffer g_buffer;
        optix::Geometry dummy;
        optix::Material material;
        std::string baseDir;

		Render::Tex2DPtr diffuse_tex;

    private:
		optix::Context minimalCreateContext();
		optix::Buffer createGBuffer();
		void createTextureSamplers( optix::Context context );
        void createInstance( optix::Context context, optix::Geometry sphere, optix::Material material );
		void pbo2Texture();
		
		Render::PBOPtr g_buffer_pbo;
		optix::TextureSampler  raster_diffuse_sampler;
		optix::TextureSampler  raster_position_sampler;
		optix::TextureSampler  raster_normal_sampler;
		void addTextureSampler(optix::TextureSampler sampler, unsigned int gl_tex_handle, float max_anisotropy, const std::string &sampler_name);
	};
}
