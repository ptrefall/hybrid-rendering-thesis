#include "Mesh.h"

#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <glm/ext.hpp>

#include <vector>


using namespace Scene;
using namespace glm;

Mesh::Mesh( const Mesh &copy )
{
	this->vao = copy.vao;
	this->vbo = copy.vbo;
	this->ibo = copy.ibo;
	this->object_to_world = copy.object_to_world;
	this->material = copy.material;
	for(auto it=copy.textures.begin(); it!=copy.textures.end(); ++it)
		setTexture(it->first, it->second.first, it->second.second, nullptr);
}

Mesh::Mesh(const Render::VAOPtr &vao, const Render::VBOPtr &vbo, const Render::IBOPtr &ibo)
	: vao(vao), vbo(vbo), ibo(ibo)
{
}

Mesh::Mesh(const std::vector<glm::vec3> &vertices, 
	       const std::vector<glm::vec3> &normals, 
	       const std::vector<glm::vec2> &tex_coords, 
		   const std::vector<unsigned int> &indices)
{
	unsigned int buffer_size = 
		sizeof(vertices[0]) * vertices.size() +
		sizeof(normals[0]) * normals.size() +
		sizeof(tex_coords[0]) * tex_coords.size();
	
	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(indices, GL_STATIC_DRAW);

	auto v_offset = vbo->buffer<glm::vec3>(vertices);
	auto n_offset = vbo->buffer<glm::vec3>(normals);
	auto t_offset = vbo->buffer<glm::vec2>(tex_coords);

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, v_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::Normal(),   3, GL_FLOAT, false, 0, n_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, t_offset);

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void Mesh::render(const Render::ShaderPtr &active_program)
{
	//object_to_world = glm::translate( position);
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