#include "parse.h"

int main()
{
	FILE* f = fopen("kitchen.aff", "r");
	if ( f==nullptr ) return 1; 
	viParseFile( f );

	return 0;
}