#pragma once

#ifndef UTILITY_BASE64_H
#define UTILITY_BASE64_H

#include <iterator>

#define QUAD1(x)	(((x) >> 2) & 0x3F)
#define QUAD2(x,y)	((((x) & 0x03) << 4) | (((y) >> 4) & 0x0F))
#define QUAD3(y,z)	((((y) & 0x0F) << 2) | (((z) >> 6) & 0x03))
#define QUAD4(z)	((z) & 0x3F)
#define TO_INT(x)   static_cast<int>(x)

namespace Utility
{

class Base64 {
public:
	~Base64()
	{}

	Base64()
	: Base64(default_b64(), default_pad())
	{}

	Base64( char const b64[], char const pad )
	: b64_(b64), pad_(pad)
	{ invert_(); }

	Base64( Base64& ) = delete;
	Base64& operator=( Base64& ) = delete;


	void setchars(  char const b64[], char const pad )
	{
		b64_ = b64; pad_ = pad;
		invert_();
	}

	void setchars( void ) /* reset */
	{
		b64_ = default_b64(); pad_ = default_pad();
		invert_();
	}

	char const* getchars( char* padptr = nullptr )
	{
		if ( padptr )
			*padptr = pad_;
		return b64_;
	}

	char getpad( char const** b64ptr = nullptr )
	{
		if ( b64ptr )
			*b64ptr = b64_;
		return pad_;
	}
		
    template<typename Iterator>
	std::size_t encode( char const* src, std::size_t len, Iterator out )
	{
        auto start = out;
		while ( len > 2 ) {
			*out++ = b64_[QUAD1(src[0])];
			*out++ = b64_[QUAD2(src[0], src[1])];
			*out++ = b64_[QUAD3(src[1], src[2])];
			*out++ = b64_[QUAD4(src[2])];
			src += 3;
			len -= 3;
		}
		switch ( len ) {
		case 2: 
			*out++ = b64_[QUAD1(src[0])];
			*out++ = b64_[QUAD2(src[0], src[1])];
			*out++ = b64_[QUAD3(src[1], 0)];
			*out++ = pad_;
			break;
		case 1:
			*out++ = b64_[QUAD1(src[0])];
			*out++ = b64_[QUAD2(src[0], 0)];
			*out++ = pad_;
			*out++ = pad_;
			break;	
		}
        return std::distance( start, out );
	}

    template<typename Iterator>
	std::size_t decode( char const* src, std::size_t len, Iterator out )
	{
        auto start = out;
        char hi;
		char lo;

		while ( len > 4 ) {
			hi      = inv_[TO_INT(*src++)] << 2; 
			lo      = inv_[TO_INT(*src++)];
			*out++  = hi | (lo >> 4);
			hi      = (lo & 0x0F) << 4; 
			lo      = inv_[TO_INT(*src++)];
			*out++  = hi | (lo >> 2);
			hi      = (lo & 0x03) << 6;
			*out++  = hi | inv_[TO_INT(*src++)];
			len    -= 4;
		}		
		
		hi     = inv_[TO_INT(*src++)] << 2; 
		lo     = inv_[TO_INT(*src++)];
		*out++ = hi | (lo >> 4);
		if ( *src != pad_ ) {
			hi     = (lo & 0x0F) << 4; 
			lo     = inv_[TO_INT(*src++)];
			*out++ = hi | (lo >> 2);
			if ( *src != pad_ ) {
				hi     = (lo & 0x03) << 6;
				*out++ = hi | inv_[TO_INT(*src)];
			}
		}
        return std::distance( start, out );
	}

	char const* getinv()
	{
		return inv_;
	}

private:
	char			inv_[256];
	char const*		b64_;
	char			pad_;

	void invert_() 
	{
		for ( int i = 0; i < 256; i++ )
			inv_[i] = 0;
		for ( int i = 0; i < 64; i++ )
			inv_[TO_INT(b64_[i])] = i;
	}
	
	char const* default_b64( )
	{
		static char const*	b64__ = 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		return b64__;
	}

	char default_pad( )
	{
		static char const pad__ = '=';
		return pad__;
	}

};

} // namespace Utility

#undef TO_INT
#undef QUAD4
#undef QUAD3
#undef QUAD2
#undef QUAD1

#endif // UTILITY_BASE64_H 
