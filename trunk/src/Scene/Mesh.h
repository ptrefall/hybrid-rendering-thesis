#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "SceneNode.h"
#include "MeshData_t.h"

#include "../File/ShaderLoader.h"

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	class Mesh : public SceneNode
	{
	public:
		Mesh(MeshDataPtr data);

		virtual void render(const Render::ShaderPtr &active_program);
		Render::VAOPtr getVao(){return vao;}
		Render::VBOPtr getVbo(){return vbo;}
		Render::IBOPtr getIbo(){return ibo;}
	protected:
		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
