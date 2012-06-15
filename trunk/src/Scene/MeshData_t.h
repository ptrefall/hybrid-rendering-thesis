#pragma once

#include <vector>

namespace Scene
{
	struct MeshData_t
	{
		std::string name; // useful when debugging
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
	typedef std::shared_ptr<MeshData_t> MeshDataPtr;

}