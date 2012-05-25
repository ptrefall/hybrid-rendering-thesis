#include "Mesh.h"

using namespace Scene;
using namespace glm;

Mesh::Mesh()
{
}

void Mesh::render(protowizard::ProtoGraphics &proto)
{
	proto.setColor( material->ambient + material->diffuse + material->specular );

	proto.setOrientation( model );
	proto.drawMesh( mesh, true );
}