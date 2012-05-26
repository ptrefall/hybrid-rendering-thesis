#include "ParseViewpoint.h"

#include "..\..\File\BARTLoader2.h"

#include <stdexcept>

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseViewpoint()
  Description:
    "v"
    "from" Fx Fy Fz
    "at" Ax Ay Az
    "up" Ux Uy Uz
    "angle" angle
    "hither" hither
    "resolution" xres yres

  Format:

    v
    from %g %g %g
    at %g %g %g
    up %g %g %g
    angle %g
    hither %g
    resolution %d %d

  The parameters are:

    From:  the eye location in XYZ.
    At:    a position to be at the center of the image, in XYZ world
	   coordinates.  A.k.a. "lookat".
    Up:    a vector defining which direction is up, as an XYZ vector.
    Angle: in degrees, defined as from the center of top pixel row to
	   bottom pixel row and left column to right column.
           In AFF, if the width is different from the height, then
           we interpret the Angle as the FOV in the y-direction (from top to bottom),
           and we set the aspect ratio to width/height.
    Resolution: in pixels, in x and in y.

  Note that no assumptions are made about normalizing the data (e.g. the
  from-at distance does not have to be 1).  Also, vectors are not
  required to be perpendicular to each other.

  For all databases some viewing parameters are always the same:
    Yon is "at infinity."
    Aspect ratio is 1.0.

  A view entity must be defined before any objects are defined (this
  requirement is so that NFF files can be used by hidden surface machines).
----------------------------------------------------------------------*/
void ParseViewpoint::parse(FILE *fp, File::BART::camera_def &cam)
{
   float hither;
   int resx;
   int resy;
	
   if(fscanf(fp, " from %f %f %f", &cam.from.x, &cam.from.y, &cam.from.z) != 3)
	   throw std::runtime_error("Parser view syntax error");
   
   if(fscanf(fp, " at %f %f %f", &cam.target.x, &cam.target.y, &cam.target.z) != 3)
      throw std::runtime_error("Parser view syntax error");
   
   if(fscanf(fp, " up %f %f %f", &cam.up.x, &cam.up.y, &cam.up.z) != 3)
      throw std::runtime_error("Parser view syntax error");
   
   if(fscanf(fp, " angle %f", &cam.fov) != 1)
      throw std::runtime_error("Parser view syntax error");
   
   if(fscanf(fp, " hither %f", &hither) !=1)
      throw std::runtime_error("Parser view syntax error");

   if(hither<0.0001) 
	   hither=1.0f;
      
   if(fscanf(fp, " resolution %d %d", &resx, &resy) !=2)
      throw std::runtime_error("Parser view syntax error");
}
