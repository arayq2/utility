
#include "HttpAgent.h"

#include <curl/curl.h>

#include <mutex>
#include <cstring>
#include <algorithm>

namespace Utility
{
namespace http
{
    namespace
    {
        std::once_flag  curlInit;
    }

    Initializer::Initializer()
    {
        std::call_once( curlInit, [](){ ::curl_global_init( CURL_GLOBAL_ALL ); } );
    }

	Agent::~Agent()
	{
		::curl_easy_cleanup( curl_ );
	}

    Agent::Agent(std::ostream& os, bool keepeol)
    : osi_(os)
    , curl_(::curl_easy_init())
    {
        set_options_( keepeol );
    }

    bool
    Agent::get( std::string const& url ) const
    {
        ::curl_easy_setopt( curl_, CURLOPT_URL, url.c_str() );
        return CURLE_OK == ::curl_easy_perform( curl_ );
    }

    Agent&
    Agent::set_headers( Headers const& headers )
    {
        set_headers_( headers );
        return *this;
    }

    std::size_t
    Agent::on_data0_( char* buf, std::size_t len )
    {
        std::copy( buf, buf + len, osi_ );
        return len;
    }

    std::size_t
    Agent::on_data1_( char* buf, std::size_t len )
    {
        std::copy_if( buf, buf + len, osi_, []( char const c ) -> bool
        {
            return ::strchr( "\r\n", c ) == nullptr;
        } );
        return len;
    }

    std::size_t /* static */
    Agent::wr_cb0( char* buf, std::size_t size, std::size_t nmemb, void* optr )
    {
        return static_cast<Agent*>(optr)->on_data0_( buf, size * nmemb );
    }

    std::size_t /* static */
    Agent::wr_cb1( char* buf, std::size_t size, std::size_t nmemb, void* optr )
    {
        return static_cast<Agent*>(optr)->on_data1_( buf, size * nmemb );
    }

    void
    Agent::set_options_( bool keepeol ) const
    {
        ::curl_easy_setopt( curl_, CURLOPT_VERBOSE, 0L );
        ::curl_easy_setopt( curl_, CURLOPT_WRITEFUNCTION, keepeol ? &wr_cb0 : &wr_cb1 );
        ::curl_easy_setopt( curl_, CURLOPT_FILE, this );        
    }

    void
    Agent::set_headers_( Headers const& headers ) const
    {
        if ( headers.empty() ) { return; }

        struct curl_slist*  _lp{nullptr};
        for ( auto& hdr : headers )
        {
            _lp = ::curl_slist_append( _lp, hdr.c_str() );
        }
        ::curl_easy_setopt( curl_, CURLOPT_HTTPHEADER, _lp );
    }

}} // namespace http, Utility
