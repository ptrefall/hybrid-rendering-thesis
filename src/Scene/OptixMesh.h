#pragma once

#include "SceneNode.h"

#include "../Render/Shader.h"
#include <Optix/optixu/optixpp_namespace.h>

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	class OptixMesh;
	typedef std::shared_ptr<OptixMesh> OptixMeshPtr;

	class OptixMesh : public SceneNode
	{
	public:
		OptixMesh(const Scene::MeshPtr &triangle_mesh, optix::Geometry &geo, optix::Material &material);
		virtual void render(const Render::ShaderPtr &active_program);
		void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) override;

		virtual void setPosition(const glm::vec3 &position) { this->position = position; updateTransform(); }
 		virtual void setOrientation(const glm::quat &orientation) { this->orientation = orientation; updateTransform(); }
 		virtual void setScale(const glm::vec3 &scale) { this->scale = scale; updateTransform(); }

		optix::Transform getTransform() {return transform;} // for top level group registration
	private:
		void updateTransform();
	private:
		optix::Geometry rtModel;
		optix::Transform transform;       // needs to be registered to top level group to render
		optix::GeometryInstance instance; // get/set geometry & material on instance
		optix::Acceleration acceleration; // needs update when transform changes
		Render::ShaderPtr boring_shader;

		Scene::MeshPtr triangle_mesh;
	};
}
