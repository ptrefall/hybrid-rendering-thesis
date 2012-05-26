#include "ParseLight.h"

#include "..\..\File\BARTLoader2.h"

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseLight()
  Positional light.  A light is defined by XYZ position and an optional color.
  For animated lights we need to be able to identify light sources. We do
  this by giving them a name.
  Description:
    "l" X Y Z [R G B]
    "la" name X Y Z [R G B]   #animated light
  Format:
    l %g %g %g [%g %g %g]
    la %s %g %g %g [%g %g %g]

    All light entities must be defined before any objects are defined (this
    requirement is so that NFF files can be used by hidden surface machines).
    Lights have a non-zero intensity of no particular value [this definition
    may change soon, with the addition of an intensity and/or color].
    The name of an animated light must not contain any white spaces.
----------------------------------------------------------------------*/
void ParseLight::parse(FILE *fp, std::vector<File::BART::light_t> &lights)
{
   glm::vec3 pos;
   glm::vec3 col;
   char name[100];
   strcpy(name,"");

   int is_animated = getc(fp);
   if(is_animated != 'a')
   {
      ungetc(is_animated, fp);
      is_animated=0;
   }

   if(is_animated)  /* if it's an animated light, read its name */
      fscanf(fp,"%s",name);

   if(fscanf(fp, "%f %f %f ",&pos.x, &pos.y, &pos.z) != 3)
	   throw std::runtime_error("Light source position syntax error");

   /* read optional color of light */
   int num=fscanf(fp, "%f %f %f ",&col.r, &col.g, &col.b);
   if(num==0)
	   col = glm::vec3(1.0f);
   else if(num!=3)
      throw std::runtime_error("Light source color syntax error");

   /* add light source here:
    * e.g. viAddLight(name,pos,col);
    */
   addLight(std::string(name),pos,col, lights);
}

void ParseLight::addLight( const std::string name, const glm::vec3& pos, const glm::vec3& col, std::vector<File::BART::light_t> &lights ) 
{
	File::BART::light_t l = {name, pos, col};
	lights.push_back( l );
}