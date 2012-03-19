#include "Quad.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <vector>

using namespace Scene;

Quad::Quad(unsigned int w, unsigned int h)
{
	unsigned int indices[] = {0,1,2, 2,3,0};			// 6
	float vertices[] = {0,0,0, w,0,0, w,h,0, 0,h,0};	// 12
	float tex_coords[] = {0,0, 1,0, 1,1, 0,1};			// 8

	unsigned int buffer_size = sizeof(float) * (12 + 8);

	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(std::vector<unsigned int>(indices,indices+6), GL_STATIC_DRAW);

	auto v_offset = vbo->buffer<float>(std::vector<float>(vertices, vertices+12));
	auto t_offset = vbo->buffer<float>(std::vector<float>(tex_coords, tex_coords+8));

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, v_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, t_offset);

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void Quad::render()
{
	vao->bind();
	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
