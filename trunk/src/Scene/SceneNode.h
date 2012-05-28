#pragma once

#include "../Render/Uniform.h"
#include "../Render/Sampler.h"
#include "../Render/Tex2D.h"
#include "../Render/Tex2DArray.h"
#include "../Render/Material.h"
#include "../Render/Shader.h"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>

namespace Scene
{
	class SceneNode;
	typedef std::shared_ptr<SceneNode> SceneNodePtr;

	class SceneNode
	{
	public:
		SceneNode();

		virtual void render(const Render::ShaderPtr &active_program) = 0;

		void setMVP(const Render::UniformPtr &mvp) { this->mvp = mvp; }
		void setMV(const Render::UniformPtr &mv) { this->mv = mv; }
		void setN_WRI(const Render::UniformPtr &n_wri) { this->n_wri = n_wri; }

		void setModelMatrix(const glm::mat4 &model) { this->model = model; }
		virtual void setPosition(const glm::vec3 &position) { this->position = position; }
		virtual void setScale(const glm::vec3 &scale) { this->scale = scale; }

		void setTexture(int slot, const Render::Tex2DPtr &tex, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler);
		virtual void setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) {}
		void setTexture(int slot, const Render::Tex2DArrayPtr &tex_array, const Render::UniformPtr &uniform, const Render::SamplerPtr &sampler);

		void setMaterial(const Render::MaterialPtr &material) { this->material = material; }

	protected:
		Render::UniformPtr mvp;
		Render::UniformPtr mv;
		Render::UniformPtr n_wri;

		std::unordered_map<int, std::pair<Render::Tex2DPtr, Render::UniformPtr>> textures;
		Render::Tex2DArrayPtr tex_array;
		Render::UniformPtr tex_uniform;
		Render::SamplerPtr tex_sampler;

		Render::MaterialPtr material;

		glm::mat4 model;
		glm::vec3 position; // TODO remove position, now that we have a mat4?
		glm::vec3 scale;
	};
}
