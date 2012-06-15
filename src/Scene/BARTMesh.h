#pragma once

#include "Mesh.h"

#include <memory>

namespace Scene
{
	struct MeshData_t;
	typedef std::shared_ptr<MeshData_t> MeshDataPtr;

	class BARTMesh;
	typedef std::shared_ptr<BARTMesh> BARTMeshPtr;

	class BARTMesh : public Mesh
	{
	public:
		BARTMesh::BARTMesh(const MeshDataPtr &meshData);
		void render(const Render::ShaderPtr &active_program) override;

		const MeshDataPtr &getMeshData() { return meshData; }
	private:
		MeshDataPtr meshData;
	};
}
