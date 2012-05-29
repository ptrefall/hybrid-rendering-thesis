#pragma once

#include "../Scene/Mesh.h"

#include <AssImp\mesh.h>
#include <AssImp\scene.h>
#include <AssImp\postprocess.h>
#include <AssImp\Importer.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Assimp { typedef std::shared_ptr<Importer> ImporterPtr; }

namespace File
{
	class MeshLoader;
	typedef std::shared_ptr<MeshLoader> MeshLoaderPtr;

	class MeshLoader
	{
	public:
		MeshLoader(const std::string &base_dir);
		~MeshLoader();

		template<class MeshType>
		std::shared_ptr<MeshType> load(const std::string &filename)
		{
			Scene::MeshDataPtr data;
			auto scene = importer->ReadFile(base_dir + filename, aiProcessPreset_TargetRealtime_Quality);
			for(unsigned int n = 0; n < scene->mNumMeshes; n++)
			{
				auto scene_mesh = scene->mMeshes[n];

				//If this is root mesh)
				if(n == 0)
				{
					data = loadMeshData(scene_mesh);
				}
				else break;
			}

			auto mesh = std::make_shared<MeshType>(data);
			return mesh;
		}

	private:
		Scene::MeshDataPtr loadMeshData(aiMesh *mesh);

		std::string base_dir;
		Assimp::ImporterPtr importer;
	};
}