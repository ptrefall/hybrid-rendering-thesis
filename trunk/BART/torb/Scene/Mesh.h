#pragma once

#include "SceneNode.h"

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
	};
}
