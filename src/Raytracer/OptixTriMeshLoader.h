#include <Optix/optixu/optixpp_namespace.h>
#include <string>

#include "../Scene/Mesh.h"

class OptixTriMeshLoader
{
public:
	struct OptixGeometryAndTriMesh_t
	{
		optix::Geometry rtGeo;
		Scene::MeshPtr triMesh;
	};

	static OptixGeometryAndTriMesh_t fromMeshData(Scene::MeshDataPtr data, optix::Context rtContext, optix::Program &isect_program, optix::Program &bbox_program);
};