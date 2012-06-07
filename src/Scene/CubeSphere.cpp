#include "CubeSphere.h"
#include "proto_camera.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"

#include <glm/ext.hpp>

#include <vector>

using namespace Scene;
using namespace glm;

CubeSphere::CubeSphere(unsigned int cell_count, float spacing)
{
	std::vector<unsigned int> indices;
	std::vector<float> vertices;
	std::vector<float> texCoords;

	unsigned int width = cell_count;
	unsigned int height = cell_count;

	unsigned int step = 1;

	unsigned int base_alloc = (width/step)*(height/step);
	vertices.reserve(	base_alloc * 3 );
	texCoords.reserve(	base_alloc * 2 );

	for(unsigned int y = 0; y < height; y += step)
	for(unsigned int x = 0; x < width; x += step)
	{
		float z = 0.0f;
		buildIndices(indices, x,y,z, width, height);
		buildVertices(vertices, x,y,z, spacing, 0.0f);
		buildTexCoords(texCoords, x,y,z, width, height);
	}

	unsigned int buffer_size = sizeof(float) * (vertices.size() + texCoords.size());

	vao = std::make_shared<Render::VAO>();
	vbo = std::make_shared<Render::VBO>(buffer_size, GL_STATIC_DRAW);
	ibo = std::make_shared<Render::IBO>(indices, GL_STATIC_DRAW);

	auto v_offset = vbo->buffer<float>(vertices);
	auto t_offset = vbo->buffer<float>(texCoords);

	Render::ATTRIB::bind(Render::ShaderConstants::Position(), 3, GL_FLOAT, false, 0, v_offset);
	Render::ATTRIB::bind(Render::ShaderConstants::TexCoord(), 2, GL_FLOAT, false, 0, t_offset);

	vao->unbind();
	vbo->unbind();
	ibo->unbind();
}

void CubeSphere::render(const Render::ShaderPtr &active_program)
{
	object_to_world = glm::translate( position);
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

	glDrawElements(GL_TRIANGLE_STRIP, ibo->size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	for(auto it=textures.begin(); it!=textures.end(); ++it)
	{
		glActiveTexture(GL_TEXTURE0+it->first);
		it->second.first->unbind();
	}
}

void CubeSphere::buildIndices(std::vector<unsigned int> &indices, unsigned int x, unsigned int y, float z, unsigned int width, unsigned int height)
{
	// 0	1	2	3
	// 4	5	6	7
	// 8	9	10	c
	// n	13	14	u
	// 16	17	18	19

	//convert the (x,y) to a single index value which represents the "current" index in the triangle strip
	//represented by "c" in the figure above
	unsigned int current_index = y*width+x;

	//find the vertex index in the grid directly under the current index
	//represented by "u" in the figure above
	unsigned int under_index = 0;
	if(y < height-1)
		under_index = (y+1)*width+x;
	else
		return; //This is the last row, which has already been covered in the previous row with triangle strips in mind

	indices.push_back(current_index);
	indices.push_back(under_index);

	//degenerate triangle technique at end of each row, so that we only have one dip call for the entire grid
	//otherwise we'd need one triangle strip per row. We only want one triangle strip for the entire grid!
	if(x < width-1)
		return;

	if(y >= height-2)
		return;

	//find the next vertex index in the grid from the current index
	//represented by "n" in the figure above
	//We already know that the current index is the last in the current row and that it's not the last row in the grid,
	//so no need to make any if-checks here.
	unsigned int next_index = (y+1)*width;

	//Add an invisible degeneration here to bind with next row in triangle strip
	indices.push_back(under_index);
	indices.push_back(next_index);
}

void CubeSphere::buildVertices(std::vector<float> &vertices, unsigned int x, unsigned int y, float z, float spacing, float height_mod)
{
	float temp_x = (float)x*spacing*2.0f - 1.0f;
	float temp_y = (float)y*spacing*2.0f - 1.0f;
	float temp_z = z*height_mod;

	float new_x = cube_to_sphere(temp_x, temp_y, temp_z);
	float new_y = cube_to_sphere(temp_y, temp_z, temp_x);
	float new_z = cube_to_sphere(temp_z, temp_x, temp_y);

	//Add one x,y,z vertex for each x,y in the grid
	vertices.push_back(new_x);
	vertices.push_back(new_y);
	vertices.push_back(new_z);
}

void CubeSphere::buildTexCoords(std::vector<float> &texCoords, unsigned int x, unsigned int y, float z, unsigned int width, unsigned int height)
{
	//Add one u,v texCoord for each x,y in the grid
	float s = ((float)x/(float)width);//*2.0f - 1.0f;
	float t = ((float)y/(float)height);//*2.0f - 1.0f;

	//s = cube_to_sphere(s,t) * 0.5f + 0.5f;
	//t = cube_to_sphere(t,s) * 0.5f + 0.5f;

	texCoords.push_back(s);
	texCoords.push_back(t);
}

float CubeSphere::cube_to_sphere(float a, float b)
{
	float b_sq = b*b;

	//http://mathproofs.blogspot.no/2005/07/mapping-cube-to-sphere.html
	return a * glm::sqrt(1.0f - (b_sq/2.0f));
}

float CubeSphere::cube_to_sphere(float a, float b, float c)
{
	float b_sq = b*b;
	float c_sq = c*c;

	//http://mathproofs.blogspot.no/2005/07/mapping-cube-to-sphere.html
	return a * glm::sqrt(1.0f - (b_sq/2.0f) - (c_sq/2.0f) + ((b_sq*c_sq)/3.0f));
}
