#ifndef B64IO_H
#define B64IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Base64.h"

#define MYBUFSZ	8192

namespace Utility
{

class B64IO {

	Base64	b64_;
	FILE*	in_;
	FILE*	out_;

public:

	B64IO( ) 
	: in_(stdin), out_(stdout) 
		{}

	B64IO( FILE* out ) 
	: in_(stdin), out_(out)
		{}

	B64IO( FILE* in, FILE* out ) 
	: in_(in), out_(out)
		{}

	~B64IO( )
		{}

	void setchars ( const char b64[], const char pad )
	{
		b64_.setchars( b64, pad );
	}

	void reset( void )
	{
		b64_.setchars();
	}

	/* encode each line of input separately */
	void encode_l( void )
	{
		char	ibuf[MYBUFSZ];
		char	obuf[MYBUFSZ+MYBUFSZ];
		char	*p;
	
		size_t	ilen;
		size_t	offs = 0;
		size_t	olen = 0;
	
		while ( (fgets( ibuf + offs, MYBUFSZ - offs, in_ )) ) {
	
			/* do we have EOL? */
			p = strpbrk( ibuf + offs, "\r\n" );
			if ( p ) {
				ilen = p - ibuf;
				offs = 0; /* no carry, we have EOL */
			}
			else { /* incomplete line */
				/* adjust ilen to multiple of 3 for encoding round */
				ilen = strlen( ibuf );
				offs = ilen % 3;
				ilen -= offs;
			}
	
			/* encode */
			olen = b64_.encode( ibuf, ilen, obuf );
			fwrite( obuf, 1, olen, out_ );

			if ( p )
				fputs( p, out_ );
			else	
			/* move unprocessed bytes to front of buffer */
			if ( offs )
				memmove( ibuf, ibuf + ilen, offs );
		}
	
		/* handle unencoded bytes, if any */
		if ( offs ) {
			olen = b64_.encode( ibuf, offs, obuf );
			fwrite( obuf, 1, olen, out_ );
		}
	}

	void encode( void )
	{
		char	ibuf[MYBUFSZ];
		char	obuf[MYBUFSZ+MYBUFSZ];
	
		size_t	ilen;
		size_t	offs = 0;
		size_t	olen = 0;
	
		while ( (ilen = fread( ibuf + offs, 1, MYBUFSZ - offs, in_ )) > 0 ) {
	
			/* adjust ilen + carry to multiple of 3 for encoding round */
			ilen += offs;
			offs  = ilen % 3;
			ilen -= offs;
	
			/* encode */
			olen = b64_.encode( ibuf, ilen, obuf );
			fwrite( obuf, 1, olen, out_ );
	
			/* move unprocessed bytes to front of buffer */
			if ( offs )
				memmove( ibuf, ibuf + ilen, offs );
		}
	
		/* handle unencoded bytes, if any */
		if ( offs ) {
			olen = b64_.encode( ibuf, offs, obuf );
			fwrite( obuf, 1, olen, out_ );
		}
	}

	/* (optional) line chunking on output */
	void encode( size_t chunk )
	{
		char	ibuf[MYBUFSZ];
		char	obuf[MYBUFSZ+MYBUFSZ];
	
		size_t	ilen;
		size_t	offs = 0;
		size_t	olen = 0;
	
		if ( chunk <= 0 ) {
			encode();
			return;
		}
	
		while ( (ilen = fread( ibuf + offs, 1, MYBUFSZ - offs, in_ )) > 0 ) {
	
			/* adjust ilen to multiple of 3 for encoding round */
			ilen += offs;
			offs = ilen % 3;
			ilen -= offs;
	
			/* encode */
			olen += b64_.encode( ibuf, ilen, obuf + olen );
	
			/* possibly partial output */
			char* p = obuf;
			while ( olen >= chunk ) {
				fwrite( p, 1, chunk, out_ ); fputs( "\n", out_ );
				olen -= chunk; p += chunk;
			}
			/* move unwritten bytes to front of buffer */
			if ( olen )
				memmove( obuf, p, olen );
	
			/* move unencoded bytes to front of buffer */
			if ( offs )
				memmove( ibuf, ibuf + ilen, offs );
		}
	
		/* handle stragglers */
	
		/* unencoded bytes, if any */
		if ( offs ) {
			olen += b64_.encode( ibuf, offs, obuf + olen );
		}
	
		/* unwritten bytes, if any */
		if ( olen ) {
			char* p = obuf;
			while ( olen >= chunk ) {
				fwrite( p, 1, chunk, out_ ); fputs( "\n", out_ );
				olen -= chunk; p += chunk;
			}
			/* possible short last line */
			if ( olen ) {
				fwrite( p, 1, olen, out_ ); fputs( "\n", out_ );
			}
		}
	}

	void encode( size_t chunk, FILE* input )
	{
		FILE* save = in_;
		in_ = input != NULL ? input : stdin;
		encode( chunk );
		in_ = save;
	}

	void encode( FILE* input )
	{
		FILE* save = in_;
		in_ = input != NULL ? input : stdin;
		encode( );
		in_ = save;
	}

	int decode( int lopt = 0 )
	{
		char	ibuf[MYBUFSZ];
		char	obuf[MYBUFSZ];
		const char*	valid;
		char*	eol;
		char	padchar;


		valid = b64_.getchars( &padchar );
	
		size_t	offs = 0;
	
		while ( fgets( ibuf + offs, MYBUFSZ - offs, in_ ) ) {
	
			/* determine length of valid input */
			size_t ilen = strspn( ibuf, valid );

			/* allow padding in the right positions */
			switch ( ilen % 4 ) {
				/* fallthrough is deliberate */
				case 2: if ( ibuf[ilen] == padchar ) ilen++;
				case 3: if ( ibuf[ilen] == padchar ) ilen++;
				default: break;
			}

			if ( lopt ) /* treat each line of input separately */
				eol = ibuf + ilen;
			else { /* adjust input length to multiple of 4 bytes */
				offs = ilen  % 4;
				ilen -= offs;
			}

			if ( ilen <= 0 )
				return -1; /* some kind of error */

			/* decode */
			size_t olen = b64_.decode( ibuf, ilen, obuf );
			fwrite( obuf, 1, olen, out_ );
	
			if ( lopt )
				fputs( eol, out_ );
			else
			/* move extra bytes to front of buffer for next round */
			if ( offs )
				memmove( ibuf, ibuf + ilen, offs );
		}
		return 0;
	}

};

} // namespace Utility

#endif //B64IO_H
