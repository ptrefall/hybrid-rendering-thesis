#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "SceneNode.h"

#include <Eigen/Eigen>
#include <memory>

namespace Scene
{
	class Cube;
	typedef std::shared_ptr<Cube> CubePtr;

	class Cube : public SceneNode
	{
	public:
		Cube(const float &size = 1.0f);

		void render(const Render::ShaderPtr &active_program) override;

	private:
		struct AvgCubeNormalsData
		{
			Eigen::Vector3f n0;
			Eigen::Vector3f n1;
			Eigen::Vector3f n2;
			Eigen::Vector3f n3;
			Eigen::Vector3f n4;
			Eigen::Vector3f n5;
			Eigen::Vector3f n6;
			Eigen::Vector3f n7;
		};
		AvgCubeNormalsData calcAvgNormalsData();

		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
