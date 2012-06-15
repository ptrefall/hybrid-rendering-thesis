#include "MeshLoader.h"

#include "..\Parser\INIParser.h"

#include <algorithm>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

using namespace File;
using namespace Assimp;

MeshLoader::MeshLoader(const std::string &base_dir)
	: base_dir(base_dir)
{
	//importer = std::make_shared<Assimp::Importer>();
	importer = new Assimp::Importer();
}

MeshLoader::~MeshLoader()
{
	delete importer;
}

Scene::MeshDataPtr MeshLoader::loadMeshData(aiMesh *mesh)
{
	auto data = std::make_shared<Scene::MeshData_t>();

	if(mesh->HasFaces())
	{
		for(unsigned int i = 0; i < mesh->mNumFaces; i++)
		for(unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			data->indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		data->vertices.push_back(mesh->mVertices[i].x);
		data->vertices.push_back(mesh->mVertices[i].y);
		data->vertices.push_back(mesh->mVertices[i].z);

		if(mesh->HasNormals())
		{
			data->normals.push_back(mesh->mNormals[i].x);
			data->normals.push_back(mesh->mNormals[i].y);
			data->normals.push_back(mesh->mNormals[i].z);
		}
		if(mesh->HasTangentsAndBitangents())
		{
			data->tangents.push_back(mesh->mTangents[i].x);
			data->tangents.push_back(mesh->mTangents[i].y);
			data->tangents.push_back(mesh->mTangents[i].z);

			data->bitangents.push_back(mesh->mBitangents[i].x);
			data->bitangents.push_back(mesh->mBitangents[i].y);
			data->bitangents.push_back(mesh->mBitangents[i].z);
		}
		for(unsigned int uvChanIndex = 0; uvChanIndex < mesh->GetNumUVChannels(); uvChanIndex++)
		{
			if(uvChanIndex == 0 && mesh->HasTextureCoords(uvChanIndex))
			{
				data->texcoords.push_back(mesh->mTextureCoords[uvChanIndex][i].x);
				data->texcoords.push_back(mesh->mTextureCoords[uvChanIndex][i].y);
			}
			else break;
		}
		for(unsigned int vColorIndex = 0; vColorIndex < mesh->GetNumColorChannels(); vColorIndex++)
		{
			if(vColorIndex == 0 && mesh->HasVertexColors(vColorIndex))
			{
				data->colors.push_back(mesh->mColors[vColorIndex][i].r);
				data->colors.push_back(mesh->mColors[vColorIndex][i].g);
				data->colors.push_back(mesh->mColors[vColorIndex][i].b);
				data->colors.push_back(mesh->mColors[vColorIndex][i].a);
			}
			else break;
		}
	}

	return data;
}
