#include "BARTMesh.h"

#include "proto_camera.h"

#include "MeshData_t.h"
#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <glm/ext.hpp>

#include <vector>


using namespace Scene;
using namespace glm;
	
BARTMesh::BARTMesh(const MeshDataPtr &meshData) 
	: Mesh(meshData)
	, meshData(meshData)
{
}

void BARTMesh::render(const Render::ShaderPtr &active_program)
{
	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	auto &view_to_clip = FirstPersonCamera::getSingleton()->getViewToClipMatrix();
	auto normal_to_view = transpose(inverse(mat3(world_to_view * object_to_world)));

	uni_object_to_world->	bind(object_to_world);
	uni_world_to_view->		bind(world_to_view);
	uni_view_to_clip->		bind(view_to_clip);
	uni_normal_to_view->	bind(normal_to_view);

	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->bind();
		it->second.second->bind((int)it->first);
	}
	
	if(material)
		material->bind_id(active_program->getFS());

	vao->bind();

	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	
	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->unbind();
	}
}