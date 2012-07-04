#include "Mesh.h"
#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <glm/ext.hpp>

#include <vector>

using namespace Scene;
using namespace glm;

Mesh::Mesh(MeshDataPtr data)
{
	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(data->getBufferSize(), GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(data->indices, GL_STATIC_DRAW);

	bool normals_loaded = false;
	bool tangents_loaded = false;
	bool bitangents_loaded = false;
	bool texcoords_loaded = false;
	bool colors_loaded = false;

	std::vector<unsigned int> offsets;
	offsets.push_back(vbo->buffer<float>(data->vertices));
	if(data->hasNormals())		offsets.push_back(vbo->buffer<float>(data->normals));
	if(data->hasTangents())		offsets.push_back(vbo->buffer<float>(data->tangents));
	if(data->hasBitangents())	offsets.push_back(vbo->buffer<float>(data->bitangents));
	if(data->hasTexCoords())	offsets.push_back(vbo->buffer<float>(data->texcoords));
	if(data->hasColors())		offsets.push_back(vbo->buffer<float>(data->colors));

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, offsets[0]);
	for(unsigned int i = 1; i < offsets.size(); i++)
	{
		if(data->hasNormals() && !normals_loaded) 
		{
			Render::ATTRIB::bind(Render::ShaderConstants::Normal(), 3, GL_FLOAT, false, 0, offsets[i]);
			normals_loaded = true;
		}
		else if(data->hasTangents() && !tangents_loaded) 
		{
			Render::ATTRIB::bind(Render::ShaderConstants::Tangent(), 3, GL_FLOAT, false, 0, offsets[i]);
			tangents_loaded = true;
		}
		else if(data->hasBitangents() && !bitangents_loaded) 
		{
			Render::ATTRIB::bind(Render::ShaderConstants::Bitangent(), 3, GL_FLOAT, false, 0, offsets[i]);
			bitangents_loaded = true;
		}
		else if(data->hasTexCoords() && !texcoords_loaded) 
		{
			Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, offsets[i]);
			texcoords_loaded = true;
		}
		else if(data->hasColors() && !colors_loaded) 
		{
			Render::ATTRIB::bind(Render::ShaderConstants::Diffuse(), 4, GL_FLOAT, false, 0, offsets[i]);
			colors_loaded = true;
		}
	}

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void Mesh::render(const Render::ShaderPtr &active_program)
{
	//object_to_world = glm::translate( position) /* * glm::mat4_cast(node_orientation)*/ * glm::scale(scale);
	auto &world_to_view = FirstPersonCamera::getSingleton()->getWorldToViewMatrix();
	auto &view_to_clip = FirstPersonCamera::getSingleton()->getViewToClipMatrix();
	auto normal_to_view = transpose(inverse(mat3(world_to_view * object_to_world)));

	uni_object_to_world->	bind(object_to_world);
	uni_world_to_view->		bind(world_to_view);
	uni_view_to_clip->		bind(view_to_clip);
	uni_normal_to_view->	bind(normal_to_view);

	int counter = 0;
	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->bind();
		it->second.second->bind((int)it->first);
		counter++;
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
