#pragma once

#include "SceneNode.h"

#include "../Render/Shader.h"
#include <Optix/optixu/optixpp_namespace.h>

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class OptixInstance;
	typedef std::shared_ptr<OptixInstance> OptixInstancePtr;

	class OptixInstance : public SceneNode
	{
	public:
		OptixInstance(optix::Geometry &geo, optix::Group &parent_group, optix::Material &optix_material);
		virtual void render(const Render::ShaderPtr &active_program);
		void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) override;

		virtual void setObjectToWorldMatrix(const glm::mat4 &object_to_world) { this->object_to_world = object_to_world; updateTransformFromMatrix(object_to_world); }


		virtual void setPosition(const glm::vec3 &position) { this->position = position; updateTransformFromPosOriScale(); }
 		virtual void setOrientation(const glm::quat &orientation) { this->orientation = orientation; updateTransformFromPosOriScale(); }
 		virtual void setScale(const glm::vec3 &scale) { this->scale = scale; updateTransformFromPosOriScale(); }

		virtual void setMaterial(const Render::MaterialPtr &material) { this->material = material; updateOptixMaterial(); }

		void removeFromScene()
		{
			transform->setChild( dummy );
			acceleration->markDirty();
		}

		void addToScene()
		{
			transform->setChild( geometrygroup );
			acceleration->markDirty();
		}

	private:
		void updateTransformFromMatrix( const glm::mat4 &m );
		void updateTransformFromPosOriScale();
		void updateOptixMaterial();
		void setupMaterial();
	private:
		optix::Geometry         rtModel;
		optix::Transform        transform; // needs to be registered to top level group to render
		optix::GeometryInstance instance;  // get/set geometry & material on instance
		optix::Material         optix_material;
		optix::GeometryGroup    geometrygroup;
		optix::Acceleration     acceleration; // needs update when transform changes
		optix::Group            parent_group; // for top level group registration

		optix::Group            dummy; // toggle rendering
	};
}
