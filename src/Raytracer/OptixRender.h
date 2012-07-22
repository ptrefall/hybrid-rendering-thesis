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

		optix::Context getContext() { return context; }; 
		void compileContext();
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

		enum ePBOType{PBO_READ, PBO_WRITE};
		optix::Buffer* g_buffer_diffuse[2];
		optix::Buffer* g_buffer_position[2];
		optix::Buffer* g_buffer_normal[2];
		
		
		Render::PBOPtr g_buffer_diffuse_pbo[2];
		Render::PBOPtr g_buffer_position_pbo[2];
		Render::PBOPtr g_buffer_normal_pbo[2];
	};
}
