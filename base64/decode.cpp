
#include "B64IO.h"

int main( int ac, char* av[] )
{
    using namespace Utility;
	B64IO	b64;
	int		lopt = 0;

	if ( ac > 1 && av[1][0] == '-' && av[1][1] == 'l' )
		lopt = 1;
	b64.decode( lopt );
	return 0;
}
		
