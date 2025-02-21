/** ======================================================================+
 + Copyright @2020-2025 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#include "AmqAgent.h"
#include "MessageHandler.h"
#include <iostream>
#include <vector>
#include <memory>

    /**
     * @class MessageReceiver
     * Implementation of a message receiver using MessageHandler.
     * This class may be used as is, or as a mixin superclass
     * that will delegate the on_message(...) call. 
     */
    template<typename CLIENT = void>
    class MessageReceiver
    {
        using Topic     = std::string;
        using Listener  = ams::MessageHandler<MessageReceiver>;
        using Listeners = std::vector<std::unique_ptr<Listener>>;
    public:

        ~MessageReceiver()
        {
            if ( release_ ) { std::cerr << "Releasing subscriptions" << std::endl; }
            for ( auto& _l : topics_ ) { agent_.unsubscribe( _l->get_info(), release_ ); }
            for ( auto& _l : queues_ ) { agent_.unsubscribe( _l->get_info(), release_ ); }
        }

        MessageReceiver(bool release = false)
        : agent_(ams::Credentials())
        , release_(release)
        {}
        // Rule of Five: move only
        MessageReceiver(MessageReceiver const&) = delete;
        MessageReceiver& operator=( MessageReceiver const& ) = delete;
        MessageReceiver(MessageReceiver&&) = default;
        MessageReceiver& operator=( MessageReceiver&& ) = default;

        void
        subscribe( std::string const& dest, bool isTopic = false )
        {
            if ( isTopic )
            {
                topics_.push_back( std::make_unique<Listener>(*this, dest) );
                agent_.subscribe( {dest, !isTopic}, topics_.back().get() );
            }
            else
            {
                queues_.push_back( std::make_unique<Listener>(*this, dest) );
                agent_.subscribe( {dest, !isTopic}, queues_.back().get() );
            }
        }

        void
        unsubscribe( std::string const& dest, bool isTopic = false )
        {
            auto    _lambda([&]( std::unique_ptr<Listener>& l ) { return l->get_info() == dest; });
            if ( isTopic ) { topics_.erase( std::remove_if( topics_.begin(), topics_.end(), _lambda ), topics_.end() ); }
            else           { queues_.erase( std::remove_if( queues_.begin(), queues_.end(), _lambda ), queues_.end() ); }
            agent_.unsubscribe( {dest, !isTopic} );
        }

        template<typename X = CLIENT>
        typename std::enable_if<std::is_void<X>::value, void>::type 
        on_message( std::string const& msg, std::string const& info )
        {
            std::cout << "[" << info << "]: ";
            std::cout << msg << "\n" << std::endl;
        }

        template<typename X = CLIENT>
        typename std::enable_if<!std::is_void<X>::value, void>::type 
        on_message( std::string const& msg, std::string const& info )
        {
            static_cast<CLIENT*>(this)->on_message( msg, info );
        }

    private:
        ams::AmqAgent   agent_;
        Listeners       topics_;
        Listeners       queues_;
        bool            release_;
    };

