#include "OptixMesh.h"
#include "proto_camera.h"
#include "Light.h"

#include "../Render/ATTRIB.h"
#include "../Render/ShaderConstants.h"


#include <glm/ext.hpp>
#include <vector>

using namespace Scene;
using namespace glm;

OptixMesh::OptixMesh(MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir)
	: Mesh(data)
{
	int num_indices = data->indices.size();
	int num_triangles = data->indices.size() / 3;
	int num_vertices = data->vertices.size() / 3;
	int num_normals = data->normals.size() / 3;

	rtModel = rtContext->createGeometry();
	rtModel->setPrimitiveCount( num_triangles );
	rtModel->setIntersectionProgram( rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_intersect" ) );
	rtModel->setBoundingBoxProgram( rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_bounds" ) );
	
	//int num_vertex_attributes = 3; // allways has verts
	//if(data->hasNormals() ) num_vertex_attributes += 3;
	//if(data->hasBitangents() ) num_vertex_attributes += 3;
	//if(data->hasTexCoords() ) num_vertex_attributes += 2;
	//if(data->hasColors() ) num_vertex_attributes += 4;

	optix::Buffer vertex_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, vbo->getHandle() );
    vertex_buffer->setFormat(RT_FORMAT_USER);
    vertex_buffer->setElementSize(3*sizeof(float));
    vertex_buffer->setSize(num_vertices + num_normals);
    rtModel["vertex_buffer"]->setBuffer(vertex_buffer);

    optix::Buffer index_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, ibo->getHandle() );
    index_buffer->setFormat(RT_FORMAT_INT3);
    index_buffer->setSize( num_triangles );
    rtModel["index_buffer"]->setBuffer(index_buffer);

	rtModel["normal_offset"]->setInt( num_normals );
	
}

void OptixMesh::init()
{
}

void OptixMesh::render(const Render::ShaderPtr &active_program)
{
}


void OptixMesh::setTexture(int slot, const Render::Tex2DPtr &tex, const std::string &uni_name) 
{ 
	//textures[slot] = std::pair<Render::Tex2DPtr, Render::UniformPtr>(tex, std::make_shared<Render::Uniform>(OptixMesh_shader->getFS(), uni_name));
}

