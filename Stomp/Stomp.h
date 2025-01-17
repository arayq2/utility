/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_STOMP_H
#define UTILITY_STOMP_H

#include <string>
#include <mutex>
#include <map>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>

namespace Stomp
{
    class Connection;

    struct EndPoint
    {
        std::string dest_;
        bool        isQ_;

        char const* prefix() const { return isQ_? "/queue/" : "/topic/"; }
    };

    struct EPComparator
    {
        bool operator()( EndPoint const& lhs, EndPoint const& rhs ) const
        {
            return lhs.dest_ < rhs.dest_;
        }
    };

    using Callback = std::function<void(std::string const&, EndPoint const&)>;

    template<typename... Args>
    Callback
    make_callback( Args... args )
    {
        return std::bind(std::forward<Args>(args)..., std::placeholders::_1, std::placeholders::_2);
    }

    /**
     * @class Session
     * @brief Maintains two components
     *        a thread-safe map of subscriptions (consumers)
     *        entry points for producers
     */
    class Session
    {
    public:
        ~Session() noexcept;
        Session(Connection&);

        bool start( EndPoint const&, Callback ); // run dispatch from outside
        bool start();                            // run dispatch internally
        bool stop();

        // true on new subscription, false on old (callback replaced)
        bool subscribe( EndPoint const& destination, Callback callback );
        // true if destination was registered
        bool unsubscribe( EndPoint const& destination );

        // true if write succeeded
        bool publish( std::string const& data, EndPoint const& destination );

    private:
        using Mutex   = std::mutex;
        using Id      = std::atomic<int>;
        using Pair    = std::pair<Callback, int>;
        using Readers = std::map<EndPoint const, Pair, EPComparator>;
        using Worker  = std::thread;
        using Boolean = std::atomic<bool>;

        Mutex       mx_;
        Connection& conn_;
        Id          id_{ATOMIC_VAR_INIT(0)};
        Readers     readers_;
        Worker      disp_;
        Boolean     started_{ATOMIC_VAR_INIT(false)};
        bool        manual_{false}; // which way were we started
        bool        stopped_{false};

        bool locate_( EndPoint const& destination, Callback& callback );
        void dispatch_();

        Session(Session const&) = delete;
        Session& operator=( Session const& ) = delete;
    };

} // namespace Stomp

#endif // UTILITY_STOMP_H
