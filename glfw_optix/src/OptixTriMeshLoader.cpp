#include "OptixTriMeshLoader.h"

//using namespace OptixTriMeshLoader;

OptixTriMeshLoader::OptixGeometryAndTriMesh_t OptixTriMeshLoader::fromMeshData(Scene::MeshDataPtr data, optix::Context rtContext, optix::Program &isect_program, optix::Program &bbox_program )
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

	rtModel->setIntersectionProgram( isect_program );
	rtModel->setBoundingBoxProgram( bbox_program );
	
	optix::Buffer vertex_buffer = rtContext->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getVbo()->getHandle() );
	vertex_buffer->setFormat(RT_FORMAT_USER);
	// actally, vertexbuffer elements are potentially larger than 3 floats, but, optix doesn't support this kind of use.
	// it works with acceleration structures like Bvh, but not SBvh and Kdtree that expect elementSize and buffer size
	// to correspond number vertices with or without stride. for these to work, one must allways fill inn all attributes 
	// (vert, norm, tangent, bitangent, texcoord), or create seperate buffers for each attribute.
	vertex_buffer->setElementSize( 3*sizeof(float));
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