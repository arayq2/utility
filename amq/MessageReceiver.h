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
            for ( auto& _l : listeners_ ) { agent_.unsubscribe( _l->get_info(), release_ ); }
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
            listeners_.push_back( std::make_unique<Listener>(*this, dest) );
            agent_.subscribe( {dest, isTopic}, listeners_.back().get() );
        }

        void
        unsubscribe( std::string const& dest, bool )
        {
            auto    _lambda([&]( std::unique_ptr<Listener>& l ) { return l->get_info() == dest; });
            listeners_.erase( std::remove_if( listeners_.begin(), listeners_.end(), _lambda ), listeners_.end() );
            agent_.unsubscribe( dest );
        }

        template<typename X = CLIENT>
        typename std::enable_if<std::is_void<X>::value, void>::type 
        on_message( std::string const& msg, std::string const& info )
        {
            if ( listeners_.size() > 1 ) { std::cout << "[" << info << "]: "; }
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
        Listeners       listeners_;
        bool            release_;
    };

