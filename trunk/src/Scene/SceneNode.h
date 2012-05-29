#pragma once

#include "../Render/Uniform.h"
#include "../Render/Sampler.h"
#include "../Render/Tex2D.h"
#include "../Render/Tex2DArray.h"
#include "../Render/Material.h"
#include "../Render/Shader.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <memory>

namespace Scene
{
	class SceneNode;
	typedef std::shared_ptr<SceneNode> SceneNodePtr;

	class SceneNode
	{
	public:
		SceneNode();

		virtual void render(const Render::ShaderPtr &active_program) = 0;

		void setObjectToWorldUniform(const Render::UniformPtr &uni_object_to_world) { this->uni_object_to_world = uni_object_to_world; }
		void setWorldToViewUniform(const Render::UniformPtr &uni_world_to_view) { this->uni_world_to_view = uni_world_to_view; }
		void setViewToClipUniform(const Render::UniformPtr &uni_view_to_clip) { this->uni_view_to_clip = uni_view_to_clip; }
		void setNormalToViewUniform(const Render::UniformPtr &uni_normal_to_view) { this->uni_normal_to_view = uni_normal_to_view; }

		void setObjectToWorldMatrix(const glm::mat4 &object_to_world) { this->object_to_world = object_to_world; }

		virtual void setPosition(const glm::vec3 &position) { this->position = position; }
		virtual void setScale(const glm::vec3 &scale) { this->scale = scale; }

		void setTexture(int slot, const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler);
		virtual void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) {}
		void setTexture(int slot, const Render::Tex2DArrayPtr &tex_array, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler);
		void setMaterial(const Render::MaterialPtr &material) { this->material = material; }

	protected:
		Render::UniformPtr uni_object_to_world;
		Render::UniformPtr uni_world_to_view;
		Render::UniformPtr uni_view_to_clip;
		Render::UniformPtr uni_normal_to_view;

		std::unordered_map<int, std::pair<Render::Tex2DPtr, Render::UniformPtr>> textures;
		Render::Tex2DArrayPtr tex_array;
		Render::UniformPtr tex_uniform;
		Render::SamplerPtr tex_sampler;

		Render::MaterialPtr material;

		glm::mat4 object_to_world;
		glm::vec3 position; // TODO remove position, now that we have a mat4?
		glm::vec3 scale;
	};
}
