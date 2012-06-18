#include "OptixTriMeshLoader.h"

//using namespace OptixTriMeshLoader;

OptixTriMeshLoader::OptixGeometryAndTriMesh_t OptixTriMeshLoader::fromMeshData(Scene::MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir)
{
	auto mesh = Scene::MeshPtr( new Scene::Mesh(data) );

	int num_indices = data->indices.size();
	int num_triangles = data->indices.size() / 3;
	int num_vertices = data->vertices.size() / 3;
	int num_normals = data->normals.size() / 3;
	int num_texCoords = data->texcoords.size() / 2;
	int numTangents = data->tangents.size() / 3;
	int numBiTangents = data->bitangents.size() / 3;

	optix::Geometry rtModel = rtContext->createGeometry();
	rtModel->setPrimitiveCount( num_triangles );
	optix::Program isect_program = rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_intersect" );
	optix::Program bbox_program = rtContext->createProgramFromPTXFile( ptx_dir+"triangle_mesh_small.cu.ptx", "mesh_bounds" );

	rtModel->setIntersectionProgram( isect_program );
	rtModel->setBoundingBoxProgram( bbox_program );
	
	optix::Buffer vertex_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getVbo()->getHandle() );
	vertex_buffer->setFormat(RT_FORMAT_USER);
	vertex_buffer->setElementSize(3*sizeof(float));
	vertex_buffer->setSize(num_vertices + num_normals + num_texCoords + numTangents + numBiTangents);
	rtModel["vertex_buffer"]->setBuffer(vertex_buffer);

	optix::Buffer index_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getIbo()->getHandle() );
	index_buffer->setFormat(RT_FORMAT_INT3);
	index_buffer->setSize( num_triangles );
	rtModel["index_buffer"]->setBuffer(index_buffer);

	rtModel["normal_offset"]->setInt( num_normals );

	OptixGeometryAndTriMesh_t out = {rtModel, mesh};
	return out;
}