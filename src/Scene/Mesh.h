#pragma once

#include "../Render/VAO.h"
#include "../Render/VBO.h"
#include "../Render/IBO.h"

#include "SceneNode.h"

#include "../File/ShaderLoader.h"

#include <glm/glm.hpp>
#include <memory>

namespace Scene
{
	struct MeshData
	{
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> tangents;
		std::vector<float> bitangents;
		std::vector<float> texcoords;
		std::vector<float> colors;
		std::vector<unsigned int> indices;

		unsigned int getBufferSize() { return sizeof(float) * (vertices.size()+normals.size()+tangents.size()+bitangents.size()+texcoords.size()+colors.size()); }
		bool hasNormals() const { return !normals.empty(); }
		bool hasTangents() const { return !tangents.empty(); }
		bool hasBitangents() const { return !bitangents.empty(); }
		bool hasTexCoords() const { return !texcoords.empty(); }
		bool hasColors() const { return !colors.empty(); }
	};
	typedef std::shared_ptr<MeshData> MeshDataPtr;

	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	class Mesh : public SceneNode
	{
	public:
		Mesh(MeshDataPtr data);

		//void render(const Render::ShaderPtr &active_program) override;

	protected:
		Render::VAOPtr vao;
		Render::VBOPtr vbo;
		Render::IBOPtr ibo;
	};
}
