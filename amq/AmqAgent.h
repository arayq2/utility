/** ======================================================================+
 + Copyright @2020-2021 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef AMS_AMQAGENT_H
#define AMS_AMQAGENT_H

#include "AmqAPI.h"

#include <string>
#include <vector>
#include <map>

    /**
     * @file AmqAgent.h
     * @brief Wrapper of ActiveMQ-CPP CMS API Session class.
     * Class supports one free producer for outbound messages
     * and multiple consumers for inbound messages.
     */

namespace ams
{

// Openwire protocol port
#define DEFAULT_AMQBROKER   "tcp://localhost:61616"

namespace
{
    char const* safe_getenv( char const* key, char const* alt )
    {
        char const* _ptr(::getenv( key ));
        return _ptr ? _ptr : alt;
    }

    char const* amqBrokerUrl(safe_getenv("AMQBROKER_URL", DEFAULT_AMQBROKER));
}
    class ExceptionLogger
    : public cms::ExceptionListener
    , public activemq::transport::DefaultTransportListener
    {
    public:
        ~ExceptionLogger() = default;
        ExceptionLogger() = default;

        void onException( const cms::CMSException& e ) override;
        void transportInterrupted() override;
        void transportResumed() override;
    };  

    struct Credentials
    {
        std::string         broker_{amqBrokerUrl};
        std::string         user_{""};
        std::string         pass_{""};
        //
        ~Credentials() = default;
        Credentials() = default;
        Credentials(std::string const& broker, std::string const& user, std::string const& pass)
        : broker_(broker)
        , user_(user)
        , pass_(pass)
        {}
        Credentials& user( std::string const& user ) { user_ = user; return *this; }
        Credentials& pass( std::string const& pass ) { pass_ = pass; return *this; }
        Credentials& uspw( std::string const& user, std::string const& pass ) { user_ = user; pass_ = pass; return *this; }
    };

    struct Receiver
    {
        cms::MessageListener*   listener_;
        ConsumerPtr             consumer_;

        ~Receiver() = default;
        Receiver() = default;
        Receiver(cms::MessageListener* listener, SessionPtr& sess, EndPoint const& ep);

        void reset( cms::MessageListener* listener );
        void reset( cms::MessageListener* listener, SessionPtr& sess, EndPoint const& ep );
    };

    class SendMap
    {
    public:
        using Target = DestinationPtr;
        using Map    = std::map<std::string const, Target>;
        //
        ~SendMap() noexcept = default;
        SendMap() = default;
        //
        Target& get( EndPoint const& dest, SessionPtr& sp );
        void drop( std::string const& dest );
        
    private:
        Map     map_;
    };

    /**
     * AmqAgent
     * @brief All-in-one simple client: one producer, multi consumer (one per subscription)
     */
    class AmqAgent
    {
        using RecvMap   = std::map<std::string const, Receiver>;
        using ConnScope = StartStop<ConnectionPtr>;
    public:
        ~AmqAgent() = default;
        
        explicit AmqAgent(Credentials const& cred);
        AmqAgent(Credentials const& cred, activemq::transport::TransportListener* transportListener, cms::ExceptionListener* exceptionListener);

        AmqAgent& subscribe( EndPoint const& endpoint, cms::MessageListener* listener );
        AmqAgent& unsubscribe( EndPoint const& endpoint, bool release = false );

        AmqAgent& publish( EndPoint const& endpoint, std::string const& text );
        AmqAgent& purge( EndPoint const& endpoint, bool keep = false ); // for cached publish targets

        std::string one_shot( std::string const& topic );

    private: // Order alert! This is for safety in destructor sequence
        Credentials    cred_;
        ConnectionPtr  conn_;
        SessionPtr     sess_;
        ConnScope      scope_;  // connection activation
        SendMap        tgts_;   // publish topic cache
        ProducerPtr    sender_; // publisher, need only one
        RecvMap        rcvrs_;  // subscribers, could have more than one
        ExceptionLogger logger_;
    };

    /**
     * Connection
     * @brief For maintenance functions that don't involve Sessions
     */
    class Connection
    {
        using ConnScope = StartStop<ConnectionPtr>;
    public:
        ~Connection() = default;
        
        explicit Connection(Credentials const& cred);

        bool remove( EndPoint const& endpoint );

    private: // Order alert! This is for safety in destructor sequence
        Credentials    cred_;
        ConnectionPtr  conn_;
        ConnScope      scope_;  // connection activation
    };

    /**
     * XConnection
     * @brief For sharing among sessions (internally)
     */
    class XConnection
    {
    public:
        ~XConnection() = default;

        explicit XConnection(Credentials const& cred);

        void start();
        void stop();

    private: // Order alert! This is for safety in destructor sequence
        Credentials    cred_;
        ConnectionPtr  conn_;
    };

} // namespace ams

#endif // AMS_AMQAGENT_H

