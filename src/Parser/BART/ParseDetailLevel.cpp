#include "ParseDetailLevel.h"

#include <stdexcept>

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
  parseDetailLevel()
  Include another file (typically containing geometry)
  Description:  
    "d" detail_level

Format:
    d %d

    The detail level (DL) number is used to exclude objects
    from the scene so that different a scene can have different
    complexities (number of primitives in them).
    The include command (i) is
    the only one that have a detail number.
    If the detail level of an included file
    is less or equal to DL then that object is included, else
    we skip it. 
    Is 0 (zero) by default.
----------------------------------------------------------------------*/
void ParseDetailLevel::parse(FILE *fp, unsigned int &detailLevel)
{
   if(fscanf(fp,"%d",&detailLevel)!=1)
	   throw std::runtime_error("Error: could not parse detail level.");
}
