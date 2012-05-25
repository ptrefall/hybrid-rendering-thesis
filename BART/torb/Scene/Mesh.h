#pragma once

#include "SceneNode.h"

//#include "../Render/VAO.h"
//#include "../Render/VBO.h"
//#include "../Render/IBO.h"

#include <glm/glm.hpp>
#include <proto/protographics.h>
#include <memory>

namespace Scene
{
	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	class Mesh : public SceneNode
	{
	public:
		Mesh();
		virtual void render(protowizard::ProtoGraphics &proto);
		void setMesh(const protowizard::MeshPtr &mesh) { this->mesh = mesh; }
	protected:
		protowizard::MeshPtr mesh;

		//Render::VAOPtr vao;
		//Render::VBOPtr vbo;
		//Render::IBOPtr ibo;
	};
}
