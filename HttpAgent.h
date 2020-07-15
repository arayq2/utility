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

        bool get( std::string const& url ) const;

        Agent& set_headers( Headers const& headers = Headers() );

    private:
        Iterator    osi_;
        void*       curl_; // typedef void* CURL

        void set_options_( bool keepeol ) const;
        void set_headers_( Headers const& ) const;

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

        std::string get() { return oss_.str(); }

        SimpleAgent& set_headers( Agent::Headers const& headers )
        {
            agent_.set_headers( headers );
            return *this;
        }

        bool operator()( std::string const& url )
        {
            oss_.str( "" );
            return agent_.get( url );
        }

    private:
        std::ostringstream  oss_;
        Agent               agent_;
    };

}} // namespace http, Utility


#endif // UTILITY_HTTP_AGENT_H
