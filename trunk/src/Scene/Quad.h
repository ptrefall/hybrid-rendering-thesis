#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "../Render/Shader.h"

#include <memory>

namespace Scene
{
	class Quad;
	typedef std::shared_ptr<Quad> QuadPtr;

	class Quad
	{
	public:
		Quad();
		void render();

	private:
		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
