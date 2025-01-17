/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/

#include "StompImpl.h"

#include <cstdio>
#include <cstring>
#include <functional>
//#include <regex>
#include <mutex>
#include <sstream>
#include <stdexcept>

#include <iostream>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

namespace Stomp
{
    using Guard  = std::lock_guard<std::mutex>;
    using Locker = std::lock_guard<Utility::WaitQueue>;

namespace
{
    std::function<void(int, char const* msg)>   OnError;

    void on_error( char const* msg, int code )
    {
        if ( OnError ) { OnError( code, msg ); }
        else { throw std::runtime_error(msg); }
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
        if ( fd_ < 0 ) { on_error( "could not create socket", -1 ); }
    }

    bool
    Socket::connect_( int port, unsigned long host )
    {
        ::memset( &addr_, 0, sizeof(addr_) );
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

namespace
{
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
                on_error( ::strerror( errno ), _nr );
            }
            return _nr;
        }
    }

    bool
    drain_( int fd, char const* buf, size_t len )
    {
        do {
            errno = 0;
            auto    _ns(::send( fd, buf, len, 0 ));
            if ( _ns < 0 )
            {
                if ( errno == EINTR ) { continue; }
                on_error( ::strerror( errno ), len );
            }
            if ( _ns == 0 ) { return false; }
            len -= _ns;
            buf += _ns;
        } while ( len > 0 );

        return true;
    }
}

// ======================================================================
    struct Frame
    {
        std::string verb_;
        std::string hdrs_;
        std::string body_;

        ~Frame() noexcept = default;
        Frame& reset() { return *this = Frame(); }
    };

    std::ostream& operator<<( std::ostream& os, Frame const& frame )
    {
        return os << frame.verb_ << frame.hdrs_ << frame.body_;
    }

    /**
     * Recvside: Frames are unmarshalled and dispatched
     */

    size_t
    Framer::fill_frame( char* start, char* end, Frame& frame )
    {
        if ( !eoh_ )
        {
            eoh_ = ::strstr( start, "\n\n" ); // end of headers
            if ( !eoh_ ) { return 0; }
        }

        // eoh_ now defined, search headers
        if ( !havecl_ )
        {
            eoh_[1] = '\0';
            char* _cl = ::strcasestr( start, "Content-Length:" );
            if ( _cl )
            {
                ::sscanf( _cl + 15, "%lu", &len_ );  // strlen( "Content-Length:" );
            }
            else { len_ = 0; } // no Content-Length header
            havecl_ = true;
        }

        if ( len_ > 0 )
        {
            if ( eoh_ + 2 + len_ >= end ) { return 0; }
            frame.body_ = std::string(eoh_ + 2, len_);
        }
        else
        {
           char* _ptr(eoh_ + 2);
           while ( *_ptr ) { ++_ptr; }
           if ( _ptr == end ) { return 0; } // we haven't found the 0-byte
           len_ = _ptr - eoh_ - 2;
           if ( len_ > 0 ) { frame.body_ = std::string(eoh_ + 2, len_); }
        }

        // verb
        char* _eol  = ::strpbrk( start, "\n" );
        frame.verb_ = std::string(start, _eol - start);
        // meta data
        frame.hdrs_ = std::string( _eol + 1, eoh_ - _eol );

        return (eoh_ - start) + len_ + 3;
    }

    bool
    Reader::read_frame( Frame& frame, int fd )
    {
        do {
            if ( !parsing_ )
            {
                size_t  _cap = BUFFERSIZE - (fillpt_ - buffer_);
                if ( _cap == 0 ) { on_error( "TOO LARGE", _cap ); }

                auto    _nr(fill_( fd, fillpt_, _cap ));

                if ( _nr == 0 ) { return false; }

                left_   += _nr;
                fillpt_ += _nr;
                *fillpt_ = '\0'; // sentinel for string operations
                parsing_ = true;
            }

            // weird hack for ActiveMQ: server responses have trailing NL(s)!
            while ( left_ > 0 && buffer_[done_] == '\n' ) { ++done_; --left_; }

            // parsing phase invariant: (fillpt_ - buffer_) == done_ + left_

            parser_.reset();  // could fine tune this, maybe
            auto    _used = left_ > 0
            ? parser_.fill_frame( buffer_ + done_, fillpt_, frame )
            : 0;

            if ( _used > 0 )
            {
                done_ += _used;
                left_ -= _used;
                return true;
            }
            else
            {
                parsing_ = false;
                if ( done_ > 0 )
                {
                    if ( left_ > 0 ) { ::memmove( buffer_, buffer_ + done_, left_ ); }
                    done_ = 0;
                }
                fillpt_ = buffer_ + left_;
            }
        } while ( true );
    }

// ======================================================================
    /**
     * Sendside: we construct Frames implicitly on the fly.
     */

    bool
    Connection::transmit_( std::string const& data )
    {
        char const* _ptr(data.data());
        size_t      _left(data.size() + 1); // yes!! The trailing NULL
        Locker      _locker(writers_);

        return drain_( sockp_->fd_, _ptr, _left );
    }

    bool
    Connection::stomp_()
    {
        std::ostringstream  _oss;
        _oss << "STOMP\naccept-version:1.0,1.1"
             << "\nhost:"
             << host_
             << "\n\n"
             << "\0";
        return transmit_( _oss.str() );
    }

namespace
{
    std::string   disconnect = "DISCONNECT\n\n\0";
}

    bool
    Connection::disconnect_()
    {
        return transmit_( disconnect );
    }

    bool
    Connection::send_( std::string const& data, EndPoint const& destination )
    {
        std::ostringstream  _oss;
        _oss << "SEND\ndestination:"
             << destination.prefix()
             << destination.dest_
             << "\n\n"
             << data
             << "\0";
        return transmit_( _oss.str() );
    }

    bool
    Connection::subscribe_( EndPoint const& destination, int id )
    {
        std::ostringstream  _oss;
        _oss << "SUBSCRIBE\ndestination:"
             << destination.prefix()
             << destination.dest_
             << "\nid:"
             << id
             << "\n\n\0";
        return transmit_( _oss.str() );
    }

    bool
    Connection::unsubscribe_( int id )
    {
        std::ostringstream  _oss;
        _oss << "UNSUBSCRIBE\nid:"
             << id
             << "\n\n\0";
        return transmit_( _oss.str() );
    }

    Connection::~Connection() = default;

    Connection::Connection(char const* host, int port)
    : sockp_(std::make_unique<Socket>())
    , host_(host)
    {
        if ( !sockp_->connect( host, port ) )
        {
            on_error( "Could not connect to host", 0 );
        }
    }

// ----------------------------------------------------------------------

    bool
    Connection::start_stomp_()
    {
        Frame   _frame;

        if ( stomped_.exchange( true ) ) { return false; }
        if ( stomp_() )
        {
            if ( reader_.read_frame( _frame, sockp_->fd_ ) )
            {
                if ( _frame.verb_.find( "CONNECTED") != std::string::npos ) { return true; }
                else {  std::cerr << _frame << std::endl; }
            }
            else { std::cerr << "Socket closed!" << std::endl; }
        }
        else { std::cerr << "Transmit failed!" << std::endl; }
        return false;
    }

    bool
    Connection::post_( Frame& frame )
    {
        if ( frame.verb_.find( "MESSAGE" ) != std::string::npos )
        {
            return true;
        }
        else
        if ( frame.verb_.find( "RECEIPT" ) != std::string::npos )
        {
            std::cerr << frame << std::endl;
        }
        else
        if ( frame.verb_.find( "ERROR" ) != std::string::npos )
        {
            std::cerr << frame << std::endl;
        }
        return false;
    }

    bool
    Connection::unmarshall_( std::string& data, EndPoint& destination, Frame const& frame )
    {
        auto    _pos1(frame.hdrs_.find( "destination:/" ));
        if ( _pos1 == std::string::npos ) { return false; }
        _pos1 += 13;  // strlen( "destination:/" )
        bool    _isq(frame.hdrs_.compare( _pos1, 5, "queue" ) == 0);
        _pos1 += 6;   // strlen( "queue/" )
        auto    _pos2(frame.hdrs_.find( "\n", _pos1 ));
        destination = {frame.hdrs_.substr( _pos1, _pos2 - _pos1 ), _isq};
        data        = frame.body_;
        return true;
    }

    bool
    Connection::receive( std::string& data, EndPoint& destination )
    {
        Frame       _frame;

        while ( true )
        {
            if ( !reader_.read_frame( _frame, sockp_->fd_ ) ) { return false; }
            if ( post_( _frame ) ) { break; }
            _frame.reset();
        }
        return unmarshall_( data, destination, _frame );
    }

// ======================================================================

    bool
    Session::start( EndPoint const& destination, Callback callback )
    {
        if ( started_.exchange( true ) ) { return false; }
        if ( !conn_.start_stomp_() )
        {
            started_.exchange( false );
            return false;
        }
        manual_ = true;
        if ( subscribe( destination, callback ) )
        {
            dispatch_();
            unsubscribe( destination );
        }
        return conn_.disconnect_();
    }

    bool
    Session::start()
    {
        if ( started_.exchange( true ) ) { return false; }
        if ( !conn_.start_stomp_() )
        {
            started_.exchange( false );
            return false;
        }
        disp_ = std::thread(&Session::dispatch_, this);
        return true;
    }

    bool
    Session::stop()
    {
        return conn_.disconnect_();
    }

    bool
    Session::subscribe( EndPoint const& destination, Callback callback )
    {
        int     _id(++id_);

        if ( conn_.subscribe_( destination, _id ) )
        {
            Guard   _guard(mx_);
            readers_.insert( {destination, {callback, _id}} );
        }
        else { return false; }
        return true;
    }

    bool
    Session::unsubscribe( EndPoint const& destination )
    {
        int     _id;
        {
            Guard   _guard(mx_);
            auto _itr = readers_.find( destination );
            if ( _itr == readers_.end() ) { return false; }
            _id = _itr->second.second;
        }
        if ( conn_.unsubscribe_( _id ) )
        {
            Guard   _guard(mx_);
            readers_.erase( destination );
            return true;
        }
        return false;
    }

    bool
    Session::publish( std::string const& data, EndPoint const& destination )
    {
        return started_.load() || start() // in case we haven't already
        ? conn_.send_( data, destination )
        : false;
    }

    bool
    Session::locate_( EndPoint const& destination, Callback& callback )
    {
        Guard   _guard(mx_);
        auto    _itr(readers_.find( destination ));
        if ( _itr != readers_.end() )
        {
            callback = _itr->second.first;
            return true;
        }
        return false;
    }

    void
    Session::dispatch_()
    {
        std::string _data;
        EndPoint    _endpoint;
        Callback    _callback;

        while ( !stopped_ )
        {
            if ( !conn_.receive( _data, _endpoint ) ) { break; }
            if ( locate_( _endpoint, _callback ) )
            {
                _callback( _data, _endpoint );
            }
        }
    }

    Session::~Session()
    {
        stop();
        stopped_ = true;
        if ( disp_.joinable() ) { disp_.join(); }
    }

    Session::Session(Connection& conn)
    : conn_(conn)
    {}

} // namespace Stomp
