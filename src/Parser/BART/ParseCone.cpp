#include "ParseCone.h"

#include "..\..\File\BARTLoader2.h"

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseCone()
  Cylinder or cone.  A cylinder is defined as having a radius and an axis
    defined by two points, which also define the top and bottom edge of the
    cylinder.  A cone is defined similarly, the difference being that the apex
    and base radii are different.  The apex radius is defined as being smaller
    than the base radius.  Note that the surface exists without endcaps.  The
    cone or cylinder description:

    "c"
    base.x base.y base.z base_radius
    apex.x apex.y apex.z apex_radius

  Format:
    c
    %g %g %g %g
    %g %g %g %g

    A negative value for both radii means that only the inside of the object is
    visible (objects are normally considered one sided, with the outside
    visible).  Note that the base and apex cannot be coincident for a cylinder
    or cone.
----------------------------------------------------------------------*/
void ParseCone::parse(FILE *fp, const Render::MaterialPtr &material, std::vector<File::BART::cone_t> &cones)
{
	glm::vec3 base_pt;
	glm::vec3 apex_pt;
	float r0,r1;
       
	if(fscanf(fp, " %f %f %f %f %f %f %f %f", &base_pt.x,&base_pt.y, &base_pt.z,&r0, &apex_pt.x,&apex_pt.y, &apex_pt.z, &r1) !=8)
		throw std::runtime_error("cylinder or cone syntax error\n");

    if(r0 < 0.0)
    {
		r0 = -r0;
		r1 = -r1;
    }

	File::BART::cone_t cone = {base_pt, apex_pt, r0, r1, material};
	cones.push_back( cone );
}