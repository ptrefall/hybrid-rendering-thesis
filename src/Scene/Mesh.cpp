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
	this->model = copy.model;
	this->material = copy.material;
	this->tex = copy.tex;
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
  auto &proj = FirstPersonCamera::getSingleton()->getProjection();
  auto &view = FirstPersonCamera::getSingleton()->getViewMatrix();

  auto modelView = view * model;
  auto modelViewProj = proj * view * model;
  auto normal = transpose(mat3(inverse(modelView)));

  mvp->bind(modelViewProj);
  mv->bind(modelView);
  n_wri->bind(normal);

	if(tex)
	{
		glActiveTexture(GL_TEXTURE0);
		tex->bind();
		if(tex_sampler)
			tex_sampler->bind(0);
		if(tex_uniform)
			tex_uniform->bind(0);
	}
	
	if(material)
		material->bind_id(active_program->getFS());

	vao->bind();

	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	
	if(tex)
		tex->unbind();
}