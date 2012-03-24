#include "Quad.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <vector>

using namespace Scene;

Quad::Quad(unsigned int w, unsigned int h)
{
	unsigned int indices[] = {0,1,2, 2,3,0};	// 6
	float vertices[] = {0,0, w,0, w,h, 0,h};	// 8

	unsigned int buffer_size = sizeof(float) * 8;

	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(std::vector<unsigned int>(indices,indices+6), GL_STATIC_DRAW);

	auto v_offset = vbo->buffer<float>(std::vector<float>(vertices, vertices+8));

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 2, GL_FLOAT, false, 0, v_offset);

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void Quad::render()
{
	vao->bind();
	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
