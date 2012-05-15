#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "SceneNode.h"

#include <glm/glm.hpp>
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
			glm::vec3 n0;
			glm::vec3 n1;
			glm::vec3 n2;
			glm::vec3 n3;
			glm::vec3 n4;
			glm::vec3 n5;
			glm::vec3 n6;
			glm::vec3 n7;
		};
		AvgCubeNormalsData calcAvgNormalsData();

		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
