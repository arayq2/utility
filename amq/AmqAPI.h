/** ======================================================================+
 + Copyright @2020-2022 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef AMS_AMQAPI_H
#define AMS_AMQAPI_H

#include "EndPoint.h"

#include <cms/TextMessage.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/commands/ActiveMQTopic.h>
#include <activemq/commands/ActiveMQQueue.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <cms/ExceptionListener.h>

    /**
     * @file AmqAPI.h
     * @brief Wrappers of the ActiveMQ-CPP CMS API.
     * This code is irredeemably ugly.
     */
namespace ams
{
    /**
     * @struct Library
     * @brief Convenience class for static initialization and release
     */
    struct Library
    {
        ~Library() noexcept { activemq::library::ActiveMQCPP::shutdownLibrary(); }
        Library() { activemq::library::ActiveMQCPP::initializeLibrary(); }
    };

    /**
     * @class StartStop
     * @brief Convenience class template to match start() and stop() calls
     */
    template<typename T>
    class StartStop
    {
        T&  tp_;
    public:
        ~StartStop() noexcept(false) { try { tp_->stop(); } catch (cms::CMSException&) {} }
        StartStop(T& tp) : tp_(tp) { tp_->start(); }
    };

//-----------------------------------------------------------------------
    /**
     * @class ConnectionPtr
     * @brief Uniqueness wrapper for CMS Connection = ActiveMQConnection 
     */
    class ConnectionPtr
    {
        std::unique_ptr<activemq::core::ActiveMQConnection> ptr_;
    public:
        ~ConnectionPtr() noexcept(false) { try { ptr_->close(); } catch (cms::CMSException&) {} }
        //
        explicit
        ConnectionPtr(std::string const& brokerURI)
        : ptr_(activemq::core::ActiveMQConnectionFactory(brokerURI).createConnection())
        {}
        // Rule of Five
        ConnectionPtr(ConnectionPtr const&) = delete;
        ConnectionPtr(ConnectionPtr&&) = default;
        ConnectionPtr& operator=( ConnectionPtr const& ) = delete;
        ConnectionPtr& operator=( ConnectionPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        activemq::core::ActiveMQConnection* operator->() { return ptr_.get(); }
        //
        void addTransportListener(activemq::transport::TransportListener* transportListener)
        {
            ptr_->addTransportListener( transportListener );
        }
        void setExceptionListener(cms::ExceptionListener* exceptionListener)
        {
            ptr_->setExceptionListener( exceptionListener );
        }
        
        bool release( EndPoint const& ep )
        {
            try {
                if ( ep.isTopic_ )
                {
                    activemq::commands::ActiveMQTopic   _topic{ep.dest_};
                    ptr_->destroyDestination( static_cast<activemq::commands::ActiveMQDestination*>(&_topic) );
                }
                else
                {
                    activemq::commands::ActiveMQQueue   _queue{ep.dest_};
                    ptr_->destroyDestination( static_cast<activemq::commands::ActiveMQDestination*>(&_queue) );
                }
                return true;
            }
            catch (activemq::exceptions::ActiveMQException&) {} // probably a snooper
            catch (cms::CMSException&) {} // @@ narrow this down?
            return false;
        }
    };

    //using ConnScope = StartStop<ConnectionPtr>;

    /**
     * @class SessionPtr
     * @brief Uniqueness wrapper for CMS Session = ActiveMQSession
     */
    class SessionPtr
    {
        std::unique_ptr<activemq::core::ActiveMQSession>  ptr_;
    public:
        ~SessionPtr() noexcept(false) { try { ptr_->close(); } catch (cms::CMSException&) {} }
        //
        explicit
        SessionPtr(ConnectionPtr& cp, cms::Session::AcknowledgeMode ackMode = cms::Session::AUTO_ACKNOWLEDGE)
        : ptr_(cp->createSession( ackMode ))
        {}
        // Rule of Five
        SessionPtr(SessionPtr const&) = delete;
        SessionPtr(SessionPtr&&) = default;
        SessionPtr& operator=( SessionPtr const& ) = delete;
        SessionPtr& operator=( SessionPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        cms::Session* operator->() { return ptr_.get(); }
    };

//-----------------------------------------------------------------------
    /**
     * @class DestinationPtr
     * @brief Uniqueness wrapper for CMS Destination  
     */
    class DestinationPtr
    {
        std::unique_ptr<cms::Destination>   ptr_;
    public:
        ~DestinationPtr() noexcept(false) {}
        //
        DestinationPtr() = default;
        // Rule of Five
        DestinationPtr(DestinationPtr const&) = delete;
        DestinationPtr(DestinationPtr&&) = default;
        DestinationPtr& operator=( DestinationPtr const& ) = delete;
        DestinationPtr& operator=( DestinationPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }

#define TODESTP(X)   static_cast<cms::Destination*>(X)
        DestinationPtr(SessionPtr& sp, EndPoint const& ep)
        : ptr_(ep.isTopic_ ? TODESTP(sp->createTopic( ep.dest_ )) : TODESTP(sp->createQueue( ep.dest_ )))
        {}
        //
        cms::Destination* get() { return ptr_.get(); }
        //
        DestinationPtr& reset( cms::Destination* dest )
        {
            ptr_.reset( dest );
            return *this;
        }
        DestinationPtr& reset( SessionPtr& sp, EndPoint const& ep )
        {
            ptr_.reset( ep.isTopic_ ? TODESTP(sp->createTopic( ep.dest_ )) : TODESTP(sp->createQueue( ep.dest_ )) );
            return *this;
        }
#undef TODESTP
    };

    /**
     * @class ConsumerPtr
     * @brief Uniqueness wrapper for CMS Consumer  
     */
    class ConsumerPtr
    {
        DestinationPtr                          dest_;
        std::unique_ptr<cms::MessageConsumer>   ptr_;
    public:
        ~ConsumerPtr() noexcept(false) { try { if ( ptr_ ) { ptr_->close(); } } catch (cms::CMSException&) {} }
        //
        ConsumerPtr() = default;
        ConsumerPtr(SessionPtr& sp, EndPoint const& ep)
        : dest_(sp, ep)
        , ptr_(sp->createConsumer( dest_.get() ))
        {}
        // Rule of Five
        ConsumerPtr(ConsumerPtr const&) = delete;
        ConsumerPtr(ConsumerPtr&&) = default;
        ConsumerPtr& operator=( ConsumerPtr const& ) = delete;
        ConsumerPtr& operator=( ConsumerPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        cms::MessageConsumer* operator->() { return ptr_.get(); }
        //
        ConsumerPtr& reset( SessionPtr& sp, EndPoint const& ep )
        {
            dest_.reset( sp, ep );
            ptr_.reset( sp->createConsumer( dest_.get() ) );
            return *this;
        }
    };

    /**
     * @class ProducerPtr
     * @brief Uniqueness wrapper for CMS Producer  
     */
    class ProducerPtr
    {
        std::unique_ptr<cms::MessageProducer>   ptr_;
    public:
        ~ProducerPtr() noexcept(false) { try { ptr_->close(); } catch (cms::CMSException&) {} }
        //
        explicit
        ProducerPtr(SessionPtr& sp)
        : ptr_(sp->createProducer())
        {}
        // Rule of Five
        ProducerPtr(ProducerPtr const&) = delete;
        ProducerPtr(ProducerPtr&&) = default;
        ProducerPtr& operator=( ProducerPtr const& ) = delete;
        ProducerPtr& operator=( ProducerPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        cms::MessageProducer* operator->() { return ptr_.get(); }
    };

    /**
     * @class BoundProducerPtr
     * @brief Uniqueness wrapper for CMS Producer bound to a Destination  
     */
    class BoundProducerPtr
    {
        DestinationPtr                          dest_;
        std::unique_ptr<cms::MessageProducer>   ptr_;
    public:
        ~BoundProducerPtr() noexcept(false) { try { ptr_->close(); } catch (cms::CMSException&) {} }
        //
        BoundProducerPtr(SessionPtr& sp, EndPoint const& ep)
        : dest_(sp, ep)
        , ptr_(sp->createProducer( dest_.get() ))
        {}
        // Rule of Five
        BoundProducerPtr(BoundProducerPtr const&) = delete;
        BoundProducerPtr(BoundProducerPtr&&) = default;
        BoundProducerPtr& operator=( BoundProducerPtr const& ) = delete;
        BoundProducerPtr& operator=( BoundProducerPtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        cms::MessageProducer* operator->() { return ptr_.get(); }
    };

    /**
     * @class TextMessagePtr
     * @brief Uniqueness wrapper for CMS TextMessage  
     */
    class TextMessagePtr
    {
        std::unique_ptr<cms::TextMessage>   ptr_;
    public:
        ~TextMessagePtr() = default;
        //
        TextMessagePtr(SessionPtr& sp, std::string const& text)
        : ptr_(sp->createTextMessage( text ))
        {}
        // Rule of Five
        TextMessagePtr(TextMessagePtr const&) = delete;
        TextMessagePtr(TextMessagePtr&&) = default;
        TextMessagePtr& operator=( TextMessagePtr const& ) = delete;
        TextMessagePtr& operator=( TextMessagePtr&& ) = default;
        //
        explicit operator bool() const { return ptr_.get() != nullptr; }
        //
        cms::TextMessage* get() { return ptr_.get(); }        
    };
} // namespace ams

#endif // AMS_AMQAPI_H
