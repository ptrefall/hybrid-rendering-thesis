#include "ParseSphere.h"

#include "..\..\File\BARTLoader2.h"

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseSphere()
  Sphere.  A sphere is defined by a radius and center position:
  
    "s" center.x center.y center.z radius

Format:
    s %g %g %g %g

    If the radius is negative, then only the sphere's inside is visible
    (objects are normally considered one sided, with the outside visible).
----------------------------------------------------------------------*/
void ParseSphere::parse(FILE *fp, const Render::MaterialPtr &material, std::vector<File::BART::sphere_t> &spheres)
{
    float radius;
    glm::vec3 center;
	
    if(fscanf(fp, "%f %f %f %f", &center.x, &center.y, &center.z, &radius) != 4)
       throw std::runtime_error("sphere syntax error");

	File::BART::sphere_t sph = {center, radius, material};
	spheres.push_back( sph );
}