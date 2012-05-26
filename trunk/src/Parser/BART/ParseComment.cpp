#include "ParseComment.h"

using namespace Parser;
using namespace BART;

/*----------------------------------------------------------------------
	parseComment()
	Description:
	"#" [ string ]
	or 
	"%" [ string ]

	Format:
	# [ string ]
	or
	% [ string ]

	As soon as a "#" (or "%") character is detected, the rest of the line is
	considered a comment. 
----------------------------------------------------------------------*/
void ParseComment::parse(FILE* f)
{
	char str[1000];
	fgets(str, 1000, f);
}
