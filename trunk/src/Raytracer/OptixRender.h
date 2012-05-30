#pragma once

#include "../Render/Tex2D.h"
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
		OptixRender(const Render::GBuffer_PassPtr &g_buffer_pass, unsigned int w, unsigned int h, const std::string& baseDir);
		void render();

		void reshape(unsigned int w, unsigned int h);

		Render::Tex2DPtr getRenderTexture() const { return tex; }

	private:
        Render::GBuffer_PassPtr g_buffer_pass;
		unsigned int w;
		unsigned int h;
        optix::Context  context; 
        optix::Geometry sphere;
        optix::Material material;
        std::string baseDir;

		Render::Tex2DPtr tex;

    private:
        optix::Context  createContext();
        optix::Material createMaterial( optix::Context context );
        optix::Geometry createGeometry( optix::Context context );
        void     createInstance( optix::Context context, optix::Geometry sphere, optix::Material material );
		void _displayFrame( optix::Buffer buffer );

	};
}
