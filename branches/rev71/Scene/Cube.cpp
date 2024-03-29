#include "Cube.h"
#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <vector>

using namespace Scene;
using namespace glm;

Cube::Cube(const float &size)
{
	unsigned int indices[] = {
								//TOP
								0,1,2, 
								2,3,0,
								//BOTTOM
								4,5,6, 
								6,7,4, 
								//FRONT
								8,9,10, 
								10,11,8, 
								//BACK
								12,13,14, 
								14,15,12, 
								//LEFT
								16,17,18,
								18,19,16, 
								//RIGHT
								20,21,22, 
								22,23,20 
							 };	// 36

	float vertices[] =	{
								//TOP
								 size,  size, -size,
								-size,  size, -size,
								-size,  size,  size,
								 size,  size,  size,
								//BOTTOM
								 size, -size,  size,
								-size, -size,  size,
								-size, -size, -size,
								 size, -size, -size,
								//FRONT
								 size,  size,  size,
								-size,  size,  size,
								-size, -size,  size,
								 size, -size,  size,
								//BACK
								 size, -size, -size,
								-size, -size, -size,
								-size,  size, -size,
								 size,  size, -size,
								//LEFT
								-size,  size,  size,
								-size,  size, -size,
								-size, -size, -size,
								-size, -size,  size,
								//RIGHT
								 size,  size, -size,
								 size,  size,  size,
								 size, -size,  size,
								 size, -size, -size
						};	// 72

	AvgCubeNormalsData n = calcAvgNormalsData();
	float normals[] =	{
								//TOP
								n.n0.x, n.n0.y, n.n0.z,
								n.n1.x, n.n1.y, n.n1.z,
								n.n2.x, n.n2.y, n.n2.z,
								n.n3.x, n.n3.y, n.n3.z,
								//BOTTOM
								n.n4.x, n.n4.y, n.n4.z,
								n.n5.x, n.n5.y, n.n5.z,
								n.n6.x, n.n6.y, n.n6.z,
								n.n7.x, n.n7.y, n.n7.z,
								//FRONT
								n.n3.x, n.n3.y, n.n3.z,
								n.n2.x, n.n2.y, n.n2.z,
								n.n5.x, n.n5.y, n.n5.z,
								n.n4.x, n.n4.y, n.n4.z,
								//BACK
								n.n7.x, n.n7.y, n.n7.z,
								n.n6.x, n.n6.y, n.n6.z,
								n.n1.x, n.n1.y, n.n1.z,
								n.n0.x, n.n0.y, n.n0.z,
								//LEFT
								n.n2.x, n.n2.y, n.n2.z,
								n.n1.x, n.n1.y, n.n1.z,
								n.n6.x, n.n6.y, n.n6.z,
								n.n5.x, n.n5.y, n.n5.z,
								//RIGHT
								n.n0.x, n.n0.y, n.n0.z,
								n.n3.x, n.n3.y, n.n3.z,
								n.n4.x, n.n4.y, n.n4.z,
								n.n7.x, n.n7.y, n.n7.z,
						};	// 72

	float tex_coords[] =	{
								//TOP
								0, 0,
								1, 0,
								1, 1,
								0, 1,
								//BOTTOM
								0, 0,
								1, 0,
								1, 1,
								0, 1,
								//FRONT
								0, 0,
								1, 0,
								1, 1,
								0, 1,
								//BACK
								0, 0,
								1, 0,
								1, 1,
								0, 1,
								//LEFT
								0, 0,
								1, 0,
								1, 1,
								0, 1,
								//RIGHT
								0, 0,
								1, 0,
								1, 1,
								0, 1,
						};	// 48

	unsigned int buffer_size = sizeof(float) * (72+72+48);

	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(std::vector<unsigned int>(indices,indices+36), GL_STATIC_DRAW);

	auto v_offset = vbo->buffer<float>(std::vector<float>(vertices, vertices+72));
	auto n_offset = vbo->buffer<float>(std::vector<float>(normals, normals+72));
	auto t_offset = vbo->buffer<float>(std::vector<float>(tex_coords, tex_coords+48));

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, v_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::Normal(),   3, GL_FLOAT, false, 0, n_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, t_offset);

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void Cube::render(const Render::ShaderPtr &active_program)
{
	mat4 model = mat4(1.0);
	model[3][0] = position.x;
	model[3][1] = position.y;
	model[3][2] = position.z;

  /*static float var = 0.f;
  var += 1e-2;
  model.rotate( AngleAxisf(var,  vec3::UnitY()) );*/
  

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
		tex_sampler->bind(0);
	}
	if(material)
		material->bind_id(active_program->getFS());

	vao->bind();

	glDrawElements(GL_TRIANGLES, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	
	if(tex)
		tex->unbind();
}

// Calculate average normals
// This is important when multiple faces will share the same vertex corner
// That is, for a cube, 3 vertices will share the same position, but for
// different faces, thus they will not share the same face normal. What's
// important then to achieve smooth light shading, is to calculate the
// average normal vector, like we do here. Note below how faces will
// share the same normal vectors for those vertices that are equal.
//////////////////////////////////////////////////////////////////////////////////
Cube::AvgCubeNormalsData Cube::calcAvgNormalsData()
{
	AvgCubeNormalsData n;

	n.n0 = vec3(0,1,0) + vec3(0,0,-1) + vec3(1,0,0);
	n.n0 /= 3.0f;
	n.n0 = normalize(n.n0);

	n.n1 = vec3(0,1,0) + vec3(0,0,-1) + vec3(-1,0,0);
	n.n1 /= 3.0f;
	n.n1 = normalize(n.n1);

	n.n2 = vec3(0,1,0) + vec3(0,0,1) + vec3(-1,0,0);
	n.n2 /= 3.0f;
	n.n2 = normalize(n.n2);

	n.n3 = vec3(0,1,0) + vec3(0,0,1) + vec3(1,0,0);
	n.n3 /= 3.0f;
	n.n3 = normalize(n.n3);

	n.n4 = vec3(0,-1,0) + vec3(0,0,1) + vec3(1,0,0);
	n.n4 /= 3.0f;
	n.n4 = normalize(n.n4);

	n.n5 = vec3(0,-1,0) + vec3(0,0,1) + vec3(-1,0,0);
	n.n5 /= 3.0f;
	n.n5 = normalize(n.n5);

	n.n6 = vec3(0,-1,0) + vec3(0,0,-1) + vec3(-1,0,0);
	n.n6 /= 3.0f;
	n.n6 = normalize(n.n6);

	n.n7 = vec3(0,-1,0) + vec3(0,0,-1) + vec3(1,0,0);
	n.n7 /= 3.0f;
	n.n7 = normalize(n.n7);

	return n;
}
