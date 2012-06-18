#include <Optix/optixu/optixpp_namespace.h>
#include <string>

#include <Scene/Mesh.h>

//struct MeshData;
//typedef std::shared_ptr<MeshData> MeshDataPtr;

class OptixTriMeshLoader
{
public:
	struct OptixGeometryAndTriMesh_t
	{
		optix::Geometry rtGeo;
		Scene::MeshPtr triMesh;
	};

	static OptixGeometryAndTriMesh_t fromMeshData(Scene::MeshDataPtr data, optix::Context rtContext, const std::string &ptx_dir);
};