#include "ParseBackground.h"

#include <stdexcept>

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
	parseBackground()
	Background color.  A color is simply RGB with values between 0 and 1
	Description:
	"b" R G B

	Format:
	b %g %g %g

	If no background color is set, assume RGB = {0,0,0}.
----------------------------------------------------------------------*/
void ParseBackground::parse(FILE* f, glm::vec3 &bgcolor)
{
	if(fscanf(f, "%f %f %f",&bgcolor.r, &bgcolor.g, &bgcolor.b) != 3)
		throw std::runtime_error("background color syntax error");
}
