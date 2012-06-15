#pragma once

#include <glm\glm.hpp>
#include <vector>
#include <stdio.h>
#include <memory>

namespace File { 
	namespace BART{ struct active_def; }
	class AssetManager; typedef std::shared_ptr<AssetManager> AssetManagerPtr;
}

namespace Scene {
	struct MeshData_t; typedef std::shared_ptr<MeshData_t> MeshDataPtr;
}

namespace Parser { namespace BART
{
	class ParseMesh
	{
	public:
		static void parse(FILE* f, const std::string &base_dir, const std::string &sceneFolder, File::BART::active_def &active, const File::AssetManagerPtr &asset_manager);

	private:
		static void getVectors(FILE *fp,char *type, std::vector<glm::vec3>& vecs);
		static void getTextureCoords(FILE *fp,char *texturename,std::vector<glm::vec2>& txts);
		static void getTriangles(FILE *fp,int *num_tris,std::vector<unsigned int>& indices, bool hasNorms, bool hasTexCoords);

		static void addMesh( const Scene::MeshDataPtr &meshData, File::BART::active_def &active, const File::AssetManagerPtr &asset_manager);
	};
}}