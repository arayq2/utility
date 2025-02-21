/** ======================================================================+
 + Copyright @2020-2025 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef AMS_MSGLISTENER_H
#define AMS_MSGLISTENER_H

#include "AmqAPI.h"
#include "Logging.h"

#include <string>
#include <vector>

    /**
     * @class MessageHandler
     * Template for async reception of ActiveMQ messages.
     * Actual client is spared the boilerplate mechanics.
     *
     * Genericity is handled through the client specified Info class
     * which is made available in the on_message(...) callback.
     * (The general idea being that each separate consumer will pass
     *  its own context-specific Info to identify message targets.)
     *
     * May need modification to adapt to logging system employed.
     */
namespace ams {

    template<typename Info>
    struct InfoToPrintable
    {   // a no-op most of the time
        static
        Info const& convert( Info const& info ) { return info; }
    };

    template<typename Client, typename Info = std::string, typename ToPrint = InfoToPrintable<Info>>
    class MessageHandler
    : public cms::MessageListener
    {
    public:
        ~MessageHandler() noexcept = default;
        MessageHandler(Client& client, Info const& info = Info())
        : cp_(&client)
        , info_(info)
        {}

        void onMessage( cms::Message const* msg ) override
        {
            auto    _tptr(dynamic_cast<cms::TextMessage const*>(msg));
            if ( _tptr )
            {
                //LOG_STRM_DEBUG(nullptr, "TextMessage [" << ToPrint::convert( info_ ) << "] received.");
                cp_->on_message( _tptr->getText(), info_ );
                return;
            }
            // fallback for ActiveMQ "smart" handling of stomp messages
            auto    _bptr(dynamic_cast<cms::BytesMessage const*>(msg));
            if ( _bptr )
            {
                LOG_STRM_DEBUG(nullptr, "BytesMessage [" << ToPrint::convert( info_ ) << "] received.");
                int                         _len(_bptr->getBodyLength());
                std::vector<unsigned char>  _vec(_len, '\0');
                _bptr->readBytes( _vec );                
                cp_->on_message( std::string(reinterpret_cast<char*>(_vec.data()), _len), info_ );
                return;
            }
            // types not handled
            if ( dynamic_cast<cms::MapMessage const*>(msg) )
            {
                LOG_STRM_ERROR(nullptr, "MapMessage received (not handled).");
                return;
            }
            if ( dynamic_cast<cms::StreamMessage const*>(msg) )
            {
                LOG_STRM_ERROR(nullptr, "StreamMessage received (not handled).");
                return;
            }
            // out of scope: ObjectMessage (Java specific)
            LOG_STRM_ERROR(nullptr, "unknown type message received (not handled).");
        }

        Info const& get_info() const { return info_; }

    private:
        Client*     cp_;
        Info        info_;
        //
    };

} // namespace ams

#endif // AMS_MSGLISTENER_H
