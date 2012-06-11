#pragma once

#include "../Scene/Mesh.h"

#include <AssImp\mesh.h>
#include <AssImp\scene.h>
#include <AssImp\postprocess.h>
#include <AssImp\Importer.hpp>

#include <memory>
#include <string>
#include <vector>

//namespace Assimp { typedef std::shared_ptr<Importer> ImporterPtr; }

namespace File
{
	class MeshLoader;
	typedef std::shared_ptr<MeshLoader> MeshLoaderPtr;

	class MeshLoader
	{
	public:
		MeshLoader(const std::string &base_dir);
		~MeshLoader();

		Scene::MeshDataPtr loadMeshDataEasy(const std::string &filename)
		{
			Scene::MeshDataPtr data;

			auto scene = importer->ReadFile(base_dir + filename, aiProcessPreset_TargetRealtime_Fast);
			if ( scene == nullptr ) {
				printf("can't find mesh %s\n", filename);
				#ifdef WIN32
					__debugbreak();
				#endif
			}
			for(unsigned int n = 0; n < scene->mNumMeshes; n++)
			{
				auto scene_mesh = scene->mMeshes[n];

				//If this is root mesh)
				if(n == 0)
				{
					data = loadMeshData(scene_mesh);
					data->name = filename;
				}
				else break;
			}

			return data;
		}

		template<class MeshType>
		std::shared_ptr<MeshType> load(const std::string &filename)
		{
			auto mesh = std::make_shared<MeshType>( loadMeshDataEasy(filename) );
			return mesh;
		}

	private:
		Scene::MeshDataPtr loadMeshData(aiMesh *mesh);

		std::string base_dir;
		Assimp::Importer *importer;
	};
}
