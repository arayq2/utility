/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_STOMPIMPL_H
#define UTILITY_STOMPIMPL_H

#include "Stomp.h"
#include "WaitQueue.h"

#include <string>
#include <map>
#include <memory>
#include <atomic>

namespace Stomp
{

    struct Socket;
    struct Frame;

    /**
     * For unmarshalling Frames
     */
    struct Framer
    {
        char*           eoh_{nullptr};  // end of headers
        size_t          len_{0};        // content length
        bool            havecl_{false}; // whether len_ is defined

        size_t fill_frame( char* start, char* end, Frame& );
        Framer& reset() { return *this = Framer(); }
    };

    /**
     * Inbound read buffer management
     */
    struct Reader
    {
        enum    { BUFFERSIZE = 100007 };
        char    buffer_[BUFFERSIZE + 1]; // extra for sentinel
        char*   fillpt_{buffer_};        // fill point for reads
        size_t  done_{0};                // alread parsed
        size_t  left_{0};                // not parsed
        Framer  parser_;                 // finds frame boundaries
        bool    parsing_{false};         // internal state

        bool read_frame( Frame& frame, int fd );
    };

    /**
     * @class Connection
     * @brief Handles I/O and protocol details.
     *        Thread-safe outbound access (producers)
     */
    class Connection
    {
    public:
        ~Connection() noexcept;
        Connection(char const* host, int port);

        bool start_stomp_();
        bool receive( std::string& data, EndPoint& destination );

        // STOMP 1.1 verbs
        bool send_( std::string const& data, EndPoint const& destination );
        bool subscribe_( EndPoint const& destination, int id );
        bool unsubscribe_( int id );
        bool disconnect_();

    private:
        using SockPtr = std::unique_ptr<Socket>;
        using Writers = Utility::WaitQueue;
        using Boolean = std::atomic<bool>;

        SockPtr     sockp_;
        std::string host_;
        //
        Writers     writers_; // serializes access to ::send()
        Reader      reader_;  // fills a Frame
        Boolean     stomped_{ATOMIC_VAR_INIT(false)}; // prevent dups

        bool stomp_();
        bool transmit_( std::string const& data );
        bool unmarshall_( std::string&, EndPoint&, Frame const& ); // for message
        bool post_( Frame& );
    };

} // namespace Stomp

#endif // UTILITY_STOMPIMPL_H
