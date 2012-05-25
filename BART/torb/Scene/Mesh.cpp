#include "Mesh.h"

using namespace Scene;
using namespace glm;

Mesh::Mesh()
{
}

void Mesh::render(protowizard::ProtoGraphics &proto)
{
	proto.setOrientation( model );
	proto.drawMesh( mesh, true );
}