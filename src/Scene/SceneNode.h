#pragma once

#include "../Render/Uniform.h"

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

		virtual void render() = 0;

		void setMVP(const Render::UniformPtr &mvp) { this->mvp = mvp; }
		void setMV(const Render::UniformPtr &mv) { this->mv = mv; }
		void setN_WRI(const Render::UniformPtr &n_wri) { this->n_wri = n_wri; }

		void setPosition(const Eigen::Vector3f &position) { this->position = position; }

	protected:
		Render::UniformPtr mvp;
		Render::UniformPtr mv;
		Render::UniformPtr n_wri;

		Eigen::Vector3f position;
		//Eigen::Transform< model;
	};
}
