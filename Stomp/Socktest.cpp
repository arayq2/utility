/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include <iostream>
#include <sstream>
#include <cstring>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>


    void fail( bool cond, char const* msg, int code )
    {
        if ( !cond )
        {
            std::cerr << msg << std::endl;
            exit( code );
        }
    }

// ======================================================================
    /**
     * Minimal implementation
     */
    struct Socket
    {
        int             fd_{0};
        ::sockaddr_in   addr_{0};
        // other stuff
        ~Socket() noexcept;
        Socket();

        bool connect( char const* host, int port );
        bool connect_( int port, unsigned long host );
    };

    Socket::~Socket()
    {
        ::close( fd_ );
    }

    Socket::Socket()
    : fd_(::socket( PF_INET, SOCK_STREAM, 0 ))
    {
        if ( fd_ < 0 ) { fail( false, "could not create socket", -1 ); }
    }

    bool
    Socket::connect_( int port, unsigned long host )
    {
        addr_.sin_family      = AF_INET;
        addr_.sin_port        = htons( (unsigned short)port );
        addr_.sin_addr.s_addr = host;
        return ::connect( fd_, (sockaddr*) &addr_, sizeof(addr_) ) == 0;
    }

    bool
    Socket::connect( char const* host, int port )
    {
        ::hostent*  _hptr(::gethostbyname( host ));

        if ( !_hptr ) { return false; }

        for ( auto _ptr((in_addr **)_hptr->h_addr_list); *_ptr; ++_ptr )
        {
            if ( connect_( port, (*_ptr)->s_addr ) ) { return true; }
        }
        return false;
    }

//-----------------------------------------------------------------------

    struct EndPoint
    {
        std::string dest_;
        bool        isQ_;

        char const* prefix() const { return isQ_? "/queue/" : "/topic/"; }
    };

    struct EPComparator
    {
        bool operator()( EndPoint const& lhs, EndPoint const& rhs ) const
        {
            return lhs.dest_ < rhs.dest_;
        }
    };

    bool
    transmit_( int fd, std::string const& data )
    {
        char const* _ptr(data.data());
        size_t      _left(data.size() + 1);
        //Locker      _locker(writers_);

        do {
            errno = 0;
            auto    _ns(::send( fd, _ptr, _left, 0 ));
            if ( _ns < 0 )
            {
                if ( errno == EINTR ) { continue; }
                fail( false, ::strerror( errno ), _left );
            }
            if ( _ns == 0 ) { return false; }
            _left -= _ns;
            _ptr  += _ns;
        } while ( _left > 0 );

        return true;
    }

    bool
    stomp_( int fd, char const* host_ )
    {
        std::ostringstream  _oss;
        _oss << "STOMP\naccept-version:1.0,1.1"
             << "\nhost:"
             << host_
             << "\n\n"
             << "\0";
        return transmit_( fd, _oss.str() );
    }

namespace
{
    std::string   disconnect = "DISCONNECT\n\n\0";
}

    bool
    disconnect_( int fd )
    {
        return transmit_( fd, disconnect );
    }

    ssize_t
    fill_( int fd, char* loc, size_t cap )
    {
        while ( true )
        {
            errno = 0;
            auto    _nr(::recv( fd, loc, cap, 0 ));

            if ( _nr < 0 )
            {
                if ( errno == EINTR ) { continue; }
                fail( false, ::strerror( errno ), _nr );
            }
            return _nr;
        }
    }

    void read_sock( int fd )
    {
        char    _buffer[100000];
        
        auto    _nr(fill_( fd, _buffer, 100000 ));
        std::cerr << "Read: " << _nr << std::endl;
        if ( _nr > 0 ) { std::cout << _nr << std::endl; }
    }


    int
    main()
    {
        Socket      _sock;
        fail( _sock.connect( "localhost", 61613 ), "Socket NOT connected", 1 );
        std::cerr << "Socket connected\n";
        fail( stomp_( _sock.fd_, "localhost" ), "stomp() failed", 2 );
        std::cerr << "stomp() succeeded\n";
        read_sock( _sock.fd_ );
        fail( disconnect_( _sock.fd_ ), "disconnect() failed", 2 );
        std::cerr << "disconnect() succeeded\n";
        return 0;
    }
