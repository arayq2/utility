/** ======================================================================+
 + Copyright @2020-2021 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_CHARBUFFER_H
#define UTILITY_CHARBUFFER_H

#include <string>
#include <cstring>
#include <cstdio>

#include <utility> // std::forward
#include <type_traits>

namespace Utility
{

    /**
     * @class CharBuffer
     * Buffer for strings formatted on the fly.
     * Specialized constructors for common creational patterns.
     *  - sprintf family
     *  - function calls
     *  - method calls
     * Note: no CharBuffer(char const*, std::size_t) constructor, by design:
     * - Formatting constructor is very greedy in overload resolution,
     *   it would dominate all implicit conversions to std::size_t!
     * - Integral value formatted into a string is a common use case.
     * - CharBuffer(int_type, char const*) constructor is an alternative.
     */
    template<std::size_t SIZE>
    class CharBuffer
    {
    public:
        ~CharBuffer() = default;

        CharBuffer()
        : buf_{0}
        {}

        CharBuffer(char const* source)
        : CharBuffer()
        {
            if ( source ) { copy_( source ); }
        }

        CharBuffer(std::string const& source)
        {
            copy_( source.c_str() );
        }

        //!> Sources larger than (SIZE - 1) are truncated.
        template<typename I, typename = typename std::enable_if<std::is_integral<I>::value>::type>
        CharBuffer(I length, char const* source)
        {
            copyx_( source, length );
        }

        void
        reset()
        {
            buf_[0] = '\0';
        }

        // ----------------------------------------------------------------

        //!> Formatting constructor. Inspiration for this class!
        template<typename... Args>
        CharBuffer(char const* fmt, Args&&... args)
        {
            format( fmt, std::forward<Args>(args)... );
        }

        // ----------------------------------------------------------------

        template<typename RV, typename... Args>
        CharBuffer(RV (*function)( char*, std::size_t, Args... ), Args&&... args)
        {
            apply( function, std::forward<Args>(args)... );
        }

        // ----------------------------------------------------------------

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ), Obj& oref, Args&&... args)
        {
            (void)(oref.*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ) const, Obj& oref, Args&&... args)
        {
            (void)(oref.*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ) const, Obj const& oref, Args&&... args)
        {
            (void)(oref.*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        // ----------------------------------------------------------------

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ), Obj* optr, Args&&... args)
        {
            (void)(optr->*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ) const, Obj* optr, Args&&... args)
        {
            (void)(optr->*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        template<typename RV, typename Obj, typename... A0s, typename... Args>
        CharBuffer(RV (Obj::* method)( char*, std::size_t, A0s... ) const, Obj const* optr, Args&&... args)
        {
            (void)(optr->*method)( buf_, SIZE, std::forward<Args>(args)... );
        }

        // ----------------------------------------------------------------

        template<typename... Args>
        CharBuffer& format( char const* fmt, Args&&... args )
        {
            ::snprintf( buf_, SIZE, fmt, std::forward<Args>(args)... );
            return *this;
        }

        template<typename Function, typename... Args>
        CharBuffer& apply( Function&& function, Args&&... args )
        {
            buf_[0] = '\0';
            function( buf_, SIZE, std::forward<Args>(args)... );
            return *this;
        }

        // ----------------------------------------------------------------

        template<std::size_t SZ>
        CharBuffer& operator=( CharBuffer<SZ> const& other )
        {
            copy_( other.get() );
            return *this;
        }

        CharBuffer& operator=( char const* source )
        {
            copy_( source );
            return *this;
        }

        CharBuffer& operator=( std::string const& source )
        {
            return operator=( source.c_str() );
        }

        CharBuffer& append( char const* tail )
        {
            char*   _end(::strchr( buf_, '\0' ));
            copy_( tail, _end - buf_ );
            return *this;
        }

        CharBuffer& append( std::string const& tail )
        {
            char*   _end(::strchr( buf_, '\0' ));
            copy_( tail.c_str(), _end - buf_ );
            return *this;
        }

        CharBuffer& keep( std::size_t len )
        {
            if ( len < SIZE ) { buf_[len] = '\0'; }
            return *this;
        }

        CharBuffer& drop( std::size_t len )
        {
            copy_( buf_ + len );
            return *this;
        }

        CharBuffer& set( char const* src, std::size_t len )
        {
            copyx_( src, len );
            return *this;
        }

        // ----------------------------------------------------------------
		int cmp( char const* str ) const { return ::strcmp( buf_, str ); }
		int ncmp( char const* str, std::size_t n ) const { return ::strncmp( buf_, str, n ); }
		int icmp( char const* str ) const { return ::strcasecmp( buf_, str ); }
		int nicmp( char const* str, std::size_t n ) const { return ::strncasecmp( buf_, str, n ); }
		char const* str(char const* str ) const { return ::strstr( buf_, str ); }
		char const* istr(char const* str ) const { return ::strcasestr( buf_, str ); }
		std::size_t spn( char const* acc ) const { return ::strspn( buf_, acc ); }
		std::size_t cspn( char const* rej ) const { return ::strcspn( buf_, rej ); }
        // ----------------------------------------------------------------

        char*       get()       { return buf_; }
        char const* get() const { return buf_; }

        std::size_t size() const { return ::strlen( buf_ ); }

        explicit operator bool() const { return buf_[0] != '\0'; }

    private:
        char    buf_[SIZE];

        void copy_( char const* src, std::ptrdiff_t offset = 0 )
        {
            char*   _dst(buf_ + offset);
            char*   _end(buf_ + SIZE - 1);
            while( *src && _dst < _end )
            {
                *_dst++ = *src++;
            }
            *_dst = '\0';
        }

        void copyx_( char const* src, std::size_t len )
        {
            std::size_t     _ext(len < SIZE ? len : SIZE - 1);
            ::memcpy( buf_, src, _ext );
            buf_[_ext] = '\0';
        }
    };

} // namespace Utility

#endif // UTILITY_CHARBUFFER_H
