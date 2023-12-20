
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

    Agent::~Agent() noexcept
    {
        free_slist_();
        ::curl_easy_cleanup( curl_ );
    }

    Agent::Agent(std::ostream& os, bool keepeol)
    : osi_(os)
    , curl_(::curl_easy_init())
    , respCode_(200)
    , lp_(nullptr)
    {
        set_options_( keepeol );
    }

    bool
    Agent::get( char const* url ) const
    {
        errbuf_[0] = '\0';
        type_ = nullptr;
        ::curl_easy_setopt( curl_, CURLOPT_HTTPGET, 1L );
        ::curl_easy_setopt( curl_, CURLOPT_URL, url );
        return perform_();
        
    }

    bool
    Agent::post( char const* url, char const* data, long size, char const* type ) const
    {
        errbuf_[0] = '\0';
        type_ = nullptr;
        ::curl_easy_setopt( curl_, CURLOPT_POSTFIELDS, const_cast<char*>(data) );
        ::curl_easy_setopt( curl_, CURLOPT_POSTFIELDSIZE, size );
        if ( type )
        {
            lp_ = ::curl_slist_append( static_cast<curl_slist*>(lp_), type );
        }
        ::curl_easy_setopt( curl_, CURLOPT_URL, url );
        return perform_();        
    }

    Agent&
    Agent::set_headers( Headers const& headers )
    {
        set_headers_( headers );
        return *this;
    }

    Agent&
    Agent::set_timeout( long timeout)
    {
        ::curl_easy_setopt( curl_, CURLOPT_TIMEOUT, timeout );
        return *this;
    }

//=========================================================================

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
        ::curl_easy_setopt( curl_, CURLOPT_ERRORBUFFER, errbuf_ );
        ::curl_easy_setopt( curl_, CURLOPT_VERBOSE, 0L );
        ::curl_easy_setopt( curl_, CURLOPT_WRITEFUNCTION, keepeol ? &wr_cb0 : &wr_cb1 );
        ::curl_easy_setopt( curl_, CURLOPT_FILE, this );        
    }

    void
    Agent::set_headers_( Headers const& headers ) const
    {
        if ( headers.empty() ) { return; }

        for ( auto& hdr : headers )
        {
            lp_ = ::curl_slist_append( static_cast<curl_slist*>(lp_), hdr.c_str() );
        }
    }

    bool
    Agent::perform_() const
    {
        if ( lp_ ) { ::curl_easy_setopt( curl_, CURLOPT_HTTPHEADER, lp_ ); }
        auto    _ok(::curl_easy_perform( curl_ ));
        ::curl_easy_getinfo( curl_, CURLINFO_RESPONSE_CODE, &respCode_ );
        //::curl_easy_getinfo( curl_, CURLINFO_SIZE_DOWNLOAD_T, &size_ );
        {
        //    double      _tmp;
              curl_off_t  _tmp;
            ::curl_easy_getinfo( curl_, CURLINFO_SIZE_DOWNLOAD_T, &_tmp );
            size_ = _tmp;
        }
        ::curl_easy_getinfo( curl_, CURLINFO_CONTENT_TYPE, &type_ );
        return CURLE_OK == _ok;
    }

    void
    Agent::free_slist_() const
    {
        if ( lp_ )
        {
            ::curl_slist_free_all( static_cast<curl_slist*>(lp_) );
            lp_ = nullptr;
        }
    }

}} // namespace http, Utility
