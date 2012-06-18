#include "ParsePoly.h"

#include "..\..\File\BARTLoader2.h"

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parsePoly()
  Polygon.  A polygon is defined by a set of vertices.  With these databases,
    a polygon is defined to have all points coplanar.  A polygon has only
    one side, with the order of the vertices being counterclockwise as you
    face the polygon (right-handed coordinate system).  The first two edges
    must form a non-zero convex angle, so that the normal and side visibility
    can be determined. 
  Description:

    "p" total_vertices
    vert1.x vert1.y vert1.z
    [etc. for total_vertices vertices]

Format:
    p %d
    [ %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------
  Polygonal patch.  A patch is defined by a set of vertices and their normals.
    With these databases, a patch is defined to have all points coplanar.
    A patch has only one side, with the order of the vertices being
    counterclockwise as you face the patch (right-handed coordinate system).
    The first two edges must form a non-zero convex angle, so that the normal
    and side visibility can be determined.  Description:

    "pp" total_vertices
    vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
    [etc. for total_vertices vertices]

Format:
    pp %d
    [ %g %g %g %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------*/
void ParsePoly::parse(FILE *fp)
{
   std::vector<glm::vec3> vertCoords;
   std::vector<glm::vec3> vertNormals;
   std::vector<glm::vec2> texCoords;

   int ispatch = getc(fp);
   if(ispatch != 'p')
   {
      ungetc(ispatch, fp);
      ispatch = 0;
   }
   
   int nverts;
   if(fscanf(fp, "%d", &nverts) != 1)
		throw std::runtime_error("polygon or patch syntax error");
	
	vertCoords.resize( nverts );
	vertNormals.resize( nverts );
	texCoords.resize( nverts );

    /* read all the vertices */
    for(int q=0; q<nverts; q++)
    {
		glm::vec3 &vertPos = vertCoords[q];
		if(fscanf(fp, " %f %f %f",&vertPos.x, &vertPos.y, &vertPos.z) != 3)
			throw std::runtime_error("polygon or patch syntax error");
       
		if(ispatch)
		{
			glm::vec3 &vertNormal = vertNormals[q];
			if(fscanf(fp, " %f %f %f",&vertNormal.x, &vertNormal.y, &vertNormal.z) != 3)
				throw std::runtime_error("polygon or patch syntax error");
		}
    }

    if(ispatch)
    {
		//add a polygon patch here
		//e.g.,  viAddPolyPatch(nverts,verts,norms);	
		addPoly( vertCoords, vertNormals, texCoords );
    }
    else
    {
		// add a polygon here
		// e.g., viAddPolygon(nverts,verts);
	
		if ( vertCoords.size() == 4 )
		{
			// TODO: does this happen? yes. polygons require som strange parsing. need to
			// generate more faces. Look at Museum scene center for an example...
			vertCoords.push_back( vertCoords[0] );
			vertCoords.push_back( vertCoords[2] );

			texCoords.push_back( texCoords[0] );
			texCoords.push_back( texCoords[2] );
		}
		// Expecting triangles, so expect num verts to be divisable by 3.
		//if( vertCoords.size() % 3 != 0 )
			//throw std::runtime_error("polygon or patch vertex count was not dividable by 3!");
		
		// need to triangulate poly, as it can be a convex hull
		// for now, only add tris
		if( vertCoords.size() % 3 ) {
			calcNormals( vertCoords, vertNormals  );
			addPoly( vertCoords, vertNormals, texCoords );
		}
    }
}

void ParsePoly::calcNormals( const std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals )
{
	// Create dummy indices for ordered triangles.
	std::vector<unsigned int> indices;
	for (size_t i=0; i<vertices.size(); i++)
		indices.push_back(i);

	calcNormals(vertices, indices, normals );
}

void ParsePoly::calcNormals(	const std::vector<glm::vec3> &vertices, 
								const std::vector<unsigned int> &indices, 
								std::vector<glm::vec3> &normals )
{
	if ( vertices.size() % 3 != 0 )
		throw std::runtime_error("not all triangles?");

	normals.resize( vertices.size() );
	for (int i=0; i<normals.size(); i++ )
		normals[i] = glm::vec3(0.f);

	for (size_t i=0; i<indices.size()/3; i++) 
	{
		const glm::vec3 &v0 = vertices[indices[3*i+0]];
		const glm::vec3 &v1 = vertices[indices[3*i+1]];
		const glm::vec3 &v2 = vertices[indices[3*i+2]];

		glm::vec3 n = glm::cross(v1 - v0, v2 - v0);
		n = glm::normalize(n);
		normals[indices[3*i+0]] += n;
		normals[indices[3*i+1]] += n;
		normals[indices[3*i+2]] += n;
	}

	for (int i=0; i<normals.size(); i++ )
		normals[i] = glm::normalize(normals[i]);
}

void ParsePoly::addPoly( const std::vector<glm::vec3> &vertCoords,
              const std::vector<glm::vec3> &vertNormals,
              const std::vector<glm::vec2> &texCoords )
{
	// TODO
	//auto mesh = std::make_shared<Scene::BARTMesh>( vertCoords, vertNormals, texCoords );
	//poly_t p = {mesh, active.tformMatrix, active.extMaterial, active.texture};
	//polyList.push_back(p);
}
