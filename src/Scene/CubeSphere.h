#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "SceneNode.h"

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class CubeSphere;
	typedef std::shared_ptr<CubeSphere> CubeSpherePtr;

	class CubeSphere : public SceneNode
	{
	public:
		CubeSphere(unsigned int cell_count = 10, float spacing = 0.1f);

		void render(const Render::ShaderPtr &active_program) override;

	private:
		void buildIndices(std::vector<unsigned int> &indices, unsigned int x, unsigned int y, float z, unsigned int width, unsigned int height);
		void buildVertices(std::vector<float> &vertices, unsigned int x, unsigned int y, float z, float spacing, float height_mod);
		void buildTexCoords(std::vector<float> &texCoords, unsigned int x, unsigned int y, float z, unsigned int width, unsigned int height);

		float cube_to_sphere(float a, float b, float c);
		float cube_to_sphere(float a, float b);

		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
