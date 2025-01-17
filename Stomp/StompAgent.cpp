/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/

#include "StompAgent.h"

namespace Stomp
{
    bool
    StompAgent::start()
    {
        return sess_.start();
    }

    bool
    StompAgent::start( EndPoint const& endpoint, Callback callback )
    {
        return sess_.start( endpoint, callback );
    }

    bool
    StompAgent::subscribe( EndPoint const& src, Callback cb )
    {
        return sess_.subscribe( src, cb );
    }

    bool
    StompAgent::unsubscribe( EndPoint const& src )
    {
        return sess_.unsubscribe( src );
    }

    bool
    StompAgent::publish( EndPoint const& tgt, std::string const& msg )
    {
        return sess_.publish( msg, tgt );
    }

    StompAgent::~StompAgent()
    {
        sess_.stop();
    }

    StompAgent::StompAgent(bool startnow, Credentials const& cred)
    : cred_(cred)
    , conn_(cred_.host_.c_str(), cred.port_)
    , sess_(conn_)
    {
        if ( startnow ) { start(); }
    }



} // namespace Stomp
