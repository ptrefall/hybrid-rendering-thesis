#include "Mesh.h"

//#include "../Render/ATTRIB.h"
//#include "../Render/ShaderConstants.h"


using namespace Scene;
using namespace glm;

Mesh::Mesh()
{
	//vao = std::make_shared<Render::VAO>();
	//vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	//ibo = std::make_shared<Render::IBO>(std::vector<unsigned int>(indices,indices+36), GL_STATIC_DRAW);

	//auto v_offset = vbo->buffer<float>(std::vector<float>(vertices, vertices+72));
	//auto n_offset = vbo->buffer<float>(std::vector<float>(normals, normals+72));
	//auto t_offset = vbo->buffer<float>(std::vector<float>(tex_coords, tex_coords+48));

	//Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, v_offset);
	//Render::ATTRIB::bind(Render::ShaderConstants::Normal(),   3, GL_FLOAT, false, 0, n_offset);
	//Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, t_offset);

	//vao->unbind();
	//vbo->unbind();
	//ibo->unbind();
}

void Mesh::render(protowizard::ProtoGraphics &proto)
{
	proto.setColor( material->ambient + material->diffuse + material->specular );

	proto.setOrientation( model );
	proto.drawMesh( mesh, true );
}