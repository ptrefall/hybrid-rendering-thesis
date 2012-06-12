#pragma once

#include "SceneNode.h"

#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>

namespace Scene
{
	class OptixNode;
	typedef std::shared_ptr<OptixNode> OptixNodePtr;

	class OptixMesh;
	typedef std::shared_ptr<OptixMesh> OptixMeshPtr;

	class OptixNode : public SceneNode
	{
	public:
		OptixNode(const OptixMeshPtr &mesh, optix::Material material);
		virtual void render(const Render::ShaderPtr &active_program);

		virtual void setPosition(const glm::vec3 &position) { this->position = position; updateTransform(); }
		virtual void setOrientation(const glm::quat &orientation) { this->orientation = orientation; updateTransform(); }
		virtual void setScale(const glm::vec3 &scale) { this->scale = scale; updateTransform(); }

		optix::Transform getTransform() {return transform;} // for top level group registration
	private:
		void updateTransform();
	private:
		OptixMeshPtr mesh;                // holds GLBO's and optix::Geometry
		optix::Transform transform;       // needs to be registered to top level group to render
		optix::GeometryInstance instance; // get/set geometry & material on instance
		optix::Acceleration acceleration; // needs update when transform changes
	};
}
