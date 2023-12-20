#pragma once

#ifndef UTILITY_HTTP_AGENT_H
#define UTILITY_HTTP_AGENT_H

#include <vector>
#include <string>
#include <iterator>
#include <iosfwd>
#include <sstream>

namespace Utility
{
namespace http
{
    struct Initializer { Initializer(); };

    class Agent
    : Initializer
    {
        using Iterator = std::ostream_iterator<char>;
    public:
        using Headers = std::vector<std::string>;
        ~Agent() noexcept;
        explicit
        Agent(std::ostream& os, bool keepeol = false);

        long get_code() const { return respCode_; }
        long get_size() const { return size_; }
        char const* get_error() const { return errbuf_; }
        char const* get_content_type() const { return type_; }

        bool get( char const* url ) const;

        // default: "Content-Type: application/x-www-forms-urlencoded"
        bool post( char const* url, char const* data, long size, char const* type = nullptr ) const;

        Agent& set_headers( Headers const& headers = Headers() );
        Agent& set_timeout( long timeout = 0 );

    private:
        Iterator        osi_;
        void*           curl_; // typedef void* CURL
        long            respCode_;
        mutable void*   lp_; // struct curl_slist*
        mutable char*   type_;
        mutable long    size_;
        mutable char    errbuf_[256]; // CURL_ERROR_SIZE

        void set_options_( bool keepeol ) const;
        void set_headers_( Headers const& ) const;
        bool perform_() const;
        void free_slist_() const;

        std::size_t on_data0_( char* buf, std::size_t len );
        std::size_t on_data1_( char* buf, std::size_t len );

        static std::size_t wr_cb0( char* buf, std::size_t size, std::size_t nmemb, void* optr );
        static std::size_t wr_cb1( char* buf, std::size_t size, std::size_t nmemb, void* optr );
    };

    class SimpleAgent
    {
    public:
        ~SimpleAgent() noexcept = default;
        explicit
        SimpleAgent(bool keepeol = false)
        : agent_(oss_, keepeol)
        {}

        explicit
        SimpleAgent(Agent::Headers const& headers, bool keepeol = false)
        : SimpleAgent(keepeol)
        {
            set_headers( headers );
        }

        long get_code() const { return agent_.get_code(); }
        long get_size() const { return agent_.get_size(); }
        char const* get_error() const { return agent_.get_error(); }
        char const* get_content_type() const { return agent_.get_content_type(); }

        std::string get_data() { return oss_.str(); }

        SimpleAgent& set_headers( Agent::Headers const& headers )
        {
            agent_.set_headers( headers );
            return *this;
        }

        SimpleAgent& set_timeout( long timeout )
        {
            agent_.set_timeout( timeout );
            return *this;
        }

        bool operator()( std::string const& url ) { return operator()( url.c_str() ); }

        bool operator()( char const* url )
        {
            oss_.str( "" );
            return agent_.get( url );
        }

        bool operator()( std::string const& url, char const* data, long size, char const* type = nullptr )
        {
            return operator()( url.c_str(), data, size, type );
        }

        bool operator()( char const* url, char const* data, long size, char const* type = nullptr )
        {
            oss_.str( "" );
            return agent_.post( url, data, size, type );
        }

    private:
        std::ostringstream  oss_;
        Agent               agent_;
    };

}} // namespace http, Utility


#endif // UTILITY_HTTP_AGENT_H
