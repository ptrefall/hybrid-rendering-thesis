#pragma once

#include "../Render/Tex2D.h"
#include "../Scene/Quad.h"
#include "../File/ShaderLoader.h"
#include <Optix/optixu/optixpp_namespace.h>

#include <memory>

namespace Render
{
    class GBuffer;
	typedef std::shared_ptr<GBuffer> GBufferPtr;
}

namespace Raytracer
{
	class OptixRender;
	typedef std::shared_ptr<OptixRender> OptixRenderRenderPtr;

	class OptixRender
	{
	public:
		OptixRender(const Render::GBufferPtr &g_buffer, unsigned int w, unsigned int h, const std::string& baseDir);
		void render();

		void reshape(unsigned int w, unsigned int h);

		Render::Tex2DPtr getRenderTexture() const { return tex; }

	private:
        Render::GBufferPtr g_buffer;
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
