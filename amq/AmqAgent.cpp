/** ======================================================================+
 + Copyright @2020-2025 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/

#include "AmqAgent.h"
#include "Finally.h"

namespace ams
{
//=========================================================================
    namespace
    {
        Library     library_; // static initialization
    }

//=========================================================================

    void ExceptionLogger::onException( const cms::CMSException& e ) 
    {
        // handle exception details here
    }

    void ExceptionLogger::transportInterrupted() 
    {        
        // handle interruption here
    }

    void ExceptionLogger::transportResumed() 
    {        
        // handle resumption here;
    }

//=========================================================================
    SendMap::Target&
    SendMap::get( EndPoint const& ep, SessionPtr& sp )
    {
        auto _itr(map_.find( ep.dest_ ));
        if ( _itr != map_.end() ) { return _itr->second; }
        return map_[ep.dest_].reset( sp, ep );
    }

    void
    SendMap::drop( std::string const& dest )
    {
        map_.erase( dest );
    }

//=========================================================================
    Receiver::Receiver(cms::MessageListener* listener, SessionPtr& sess, EndPoint const& ep)
    : listener_(listener)
    , consumer_(sess, ep)
    {
        consumer_->setMessageListener( listener_ );
    }

    void
    Receiver::reset( cms::MessageListener* listener )
    {
        listener_ = listener;
        consumer_->setMessageListener( listener_ );
    }

    void
    Receiver::reset( cms::MessageListener* listener, SessionPtr& sess, EndPoint const& ep )
    {
        consumer_.reset( sess, ep );
        reset( listener );
    }

//=========================================================================
    AmqAgent::AmqAgent(Credentials const& cred)
    : cred_(cred)
    , conn_(cred_.broker_)
    , sess_(conn_)
    , scope_(conn_)
    , sender_(sess_)
    {
        if ( conn_ )
        {
            conn_->addTransportListener( &logger_ );
            conn_->setExceptionListener( &logger_ );
        }
    }

    AmqAgent&
    AmqAgent::unsubscribe( EndPoint const& ep, bool release )
    {
        subscribe( ep, nullptr );
        if ( release ) { conn_.release( ep ); }
        return *this;
    }

    AmqAgent&
    AmqAgent::subscribe( EndPoint const& ep, cms::MessageListener* listener )
    {
        if ( listener )
        {
            auto    _itr(rcvrs_.find( ep.dest_ ));
            if ( _itr == rcvrs_.end() )
            {
                rcvrs_[ep.dest_].reset( listener, sess_, ep );
            }
            else { _itr->second.reset( listener ); }
        }
        else { rcvrs_.erase( ep.dest_ ); }
        return *this;
    }

    AmqAgent&
    AmqAgent::publish( EndPoint const& ep, std::string const& text )
    {   // demonstrates the utter silliness of Java-izing a C++ API
        DestinationPtr&    _dest(tgts_.get( ep, sess_ ));
        TextMessagePtr     _msg(sess_, text);
        sender_->send( _dest.get(), _msg.get() );
        return *this;
    }

    AmqAgent&
    AmqAgent::purge( EndPoint const& ep, bool keep )
    {
        tgts_.drop( ep.dest_ );
        if ( !keep ) { conn_.release( ep ); }
        return *this;
    }

    std::string
    AmqAgent::one_shot( std::string const& topic )
    {
        ConsumerPtr         _cp(sess_, {topic, false});
        auto                _mp(_cp->receiveNoWait());
        if ( !_mp ) { return ""; }
        Utility::Finally    _fin{[_mp]() { delete _mp; }};
        // this is a disgusting API: relying on dynamic_cast for type of Message
        auto    _tptr(dynamic_cast<cms::TextMessage const*>(_mp));
        if ( _tptr )
        {
            return _tptr->getText();
        }
        // fallback for ActiveMQ "smart" handling of stomp messages
        auto    _bptr(dynamic_cast<cms::BytesMessage const*>(_mp));
        if ( _bptr )
        {
            int                         _len(_bptr->getBodyLength());
            std::vector<unsigned char>  _vec(_len, '\0');
            _bptr->readBytes( _vec );
            return std::string(reinterpret_cast<char*>(_vec.data()), _len);
        }
        // give up
        return "";
    }

//=========================================================================
    Connection::Connection(Credentials const& cred)
    : cred_(cred)
    , conn_(cred_.broker_)
    , scope_(conn_)
    {}

    bool
    Connection::remove( EndPoint const& ep )
    {
        return conn_ ? conn_.release( ep ) : false;
    }

} // namespace ams
