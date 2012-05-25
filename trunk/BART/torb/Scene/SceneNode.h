#pragma once

#include "../Render/Material.h"

#include <glm/glm.hpp>
#include <proto/protographics.h>
#include <memory>

namespace Scene
{
	class SceneNode;
	typedef std::shared_ptr<SceneNode> SceneNodePtr;

	class SceneNode
	{
	public:
		SceneNode();

		virtual void render(protowizard::ProtoGraphics &proto) = 0;

		void setModelMatrix(const glm::mat4 &model) { this->model = model; }
		//void setTexture(const Render::Tex2DArrayPtr &tex_array, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler);
		void setMaterial(const Render::MaterialPtr &material) { this->material = material; }

	protected:
		// tex
		// material
		// model matrix


		Render::MaterialPtr material;
		glm::mat4 model;
	};
}
