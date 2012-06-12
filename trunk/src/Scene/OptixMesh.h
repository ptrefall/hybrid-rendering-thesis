#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "Mesh.h"

#include "../Render/Shader.h"
#include <Optix/optixu/optixpp_namespace.h>

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class OptixMesh;
	typedef std::shared_ptr<OptixMesh> OptixMeshPtr;

	class OptixMesh : public Mesh
	{
	public:
		OptixMesh(MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir);
		virtual void render(const Render::ShaderPtr &active_program);
		void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) override;

		optix::Geometry getGeometry() const { return rtModel; };
	private:
		optix::Geometry rtModel;
		Render::ShaderPtr boring_shader;
	};
}
