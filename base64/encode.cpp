
#include "B64IO.h"

int main( int ac, char* av[] )
{
    using namespace Utility;
	B64IO	b64;
	int 	chunksize = 0;

	if ( ac > 1 ) {
		if ( av[1][0] == '-' ) {
			if (av[1][1] == 'l' ) {
				b64.encode_l( );
				return 0;
			}
				
		}
		else 
			chunksize = atoi( av[1] );
		if ( chunksize < 0 )
			chunksize = 0;
	}

	b64.encode( (size_t) chunksize );
	return 0;
}
