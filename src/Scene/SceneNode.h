#pragma once

#include "../Render/Uniform.h"
#include "../Render/Sampler.h"
#include "../Render/Tex2D.h"
#include "../Render/Tex2DArray.h"
#include "../Render/Material.h"
#include "../Render/Shader.h"

#include <Eigen/Eigen>
#include <Eigen/Geometry>

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

		void setMVP(const Render::UniformPtr &mvp) { this->mvp = mvp; }
		void setMV(const Render::UniformPtr &mv) { this->mv = mv; }
		void setN_WRI(const Render::UniformPtr &n_wri) { this->n_wri = n_wri; }

		void setPosition(const Eigen::Vector3f &position) { this->position = position; }

		void setTexture(const Render::Tex2DPtr &tex, const Render::SamplerPtr &sampler);
		void setTexture(const Render::Tex2DArrayPtr &tex_array, const Render::SamplerPtr &sampler);
		void setMaterial(const Render::MaterialPtr &material) { this->material = material; }

	protected:
		Render::UniformPtr mvp;
		Render::UniformPtr mv;
		Render::UniformPtr n_wri;

		Render::Tex2DPtr tex;
		Render::Tex2DArrayPtr tex_array;
		Render::SamplerPtr tex_sampler;

		Render::MaterialPtr material;

		Eigen::Vector3f position;
		//Eigen::Transform< model;
	};
}
