/** ======================================================================+
 + Copyright @2019-2022 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_SCHEDULER_H
#define UTILITY_SCHEDULER_H

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <algorithm>

namespace Utility
{
    using SysClock  = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<SysClock>;
    using MilliSecs = std::chrono::milliseconds;

    /**
     * @class Scheduler
     * @brief Simple scheduler, templated on handler class
     */
    template<typename Handler>
    class Scheduler
    {
        //reduce type clutter
        using Item      = typename Handler::item_type;
        using Map       = std::multimap<TimePoint, Item>; // substitute for a treap
        using Value     = typename Map::value_type;
        using List      = std::vector<Value>; // for cancelled items
        using Mutex     = std::mutex;
        using Guard     = std::lock_guard<Mutex>;
        using Lock      = std::unique_lock<Mutex>;
        using Condition = std::condition_variable;
        using Thread    = std::thread;
    public:

        ~Scheduler() noexcept;
        explicit Scheduler(Handler&& handler, long millis = 0);

        void stop();
        std::size_t size(); // const;
        TimePoint schedule( Item& item ); // default timeout
        TimePoint schedule( Item& item, long millis );
        std::size_t cancel( TimePoint tp, Item& item );

        /**
         * @class Watch
         * @brief Sentinel: calls schedule() in ctor, cancel() in dtor
         */
        class Watch
        {
        public:
            ~Watch() noexcept { if ( !keep_ ) { cancel(); } }
            Watch(Scheduler& scheduler, Item& item, long millis)
            : scheduler_(scheduler)
            , item_(item)
            , tp_(scheduler_.schedule( item_, millis ))
            {}
            Watch(Scheduler& scheduler, Item& item)
            : scheduler_(scheduler)
            , item_(item)
            , tp_(scheduler_.schedule( item_ ))
            {}

            void cancel() { scheduler_.cancel( tp_, item_ ); }
            bool keep( bool val ) { return (keep_ = val); }

        private:
            Scheduler&  scheduler_;
            Item&       item_;
            TimePoint   tp_;
            bool        keep_{false};

            Watch(Watch const&) = delete;
            Watch& operator=( Watch const& ) = delete;
        };

        /**
         * @class TimeOut
         * @brief Countdown Timer: invokes handler on timeout expiry unless cancelled.
         * Setting default duration to zero via ctor or reinit() disables the timeout.
         */
        class TimeOut
        {
        public:
            ~TimeOut() noexcept = default;
            TimeOut() = default;
            explicit
            TimeOut(Scheduler& scheduler, long millis = 0)
            : sptr_(&scheduler)
            , millis_(millis > 0 ? millis : 0)
            {}

            // changed return type from void to long by PM 5/23 for logging
            long set( Item const& item ) { return set( item, millis_ ); }
            long set( Item const& item, long millis )
            {
                if ( millis <= 0L ) { return 0L; }
                item_ = item;
                tp_   = sptr_->schedule( item_, millis );
                return millis;
            }

            void cancel() { sptr_->cancel( tp_, item_ ); }

            void renew()
            {
                cancel();
                tp_ = sptr_->schedule( item_, millis_ );
            }

            long timeleft()
            {
                long    _tl(std::chrono::duration_cast<MilliSecs>(tp_ - SysClock::now()).count());
                return _tl < 0 ? millis_ : _tl;
            }

            // changed return type from void to long by PM 5/23 for logging
            long extend( long millis )
            {   // not exact, but will suffice
                if ( millis <= 0 ) 
                { 
                    return timeleft();
                }

                cancel();
                long _newTime = millis + timeleft();
                tp_ = sptr_->schedule( item_, _newTime );
                return _newTime;
            }

            TimeOut& reinit( long millis )
            {
                if ( millis >= 0 ) { millis_ = millis; }
                return *this;
            }

        private:
            Scheduler*  sptr_{nullptr};
            long        millis_{0};
            Item        item_;
            TimePoint   tp_;

            //TimeOut(TimeOut const&) = delete;
            //TimeOut& operator=( TimeOut const& ) = delete;
        };

        struct Restarter
        {
            TimeOut&    to_;
            Item        item_;
            ~Restarter()
            {
                to_.set( item_ );
            }
            Restarter(TimeOut& to, Item const& item)
            : to_(to)
            , item_(item)
            {
                to_.cancel();
            }
        };

    private:
        bool        stopped_{false};
        Mutex       mx_;
        Condition   ready_;   // timeout signal
        Map         map_;     // items to be activated, in order by TimePoint
        List        list_;    // items to be purged
        Handler     handler_; // item activator
        long        millis_;  // default timeout
        Thread      runner_;  // thread of control for Handler

        bool is_active_( Value const& ) const;
        void run_();
        void purge_(); // under lock!
    };

    template<typename Handler>
    inline
    Scheduler<Handler>::~Scheduler() noexcept
    {
        stop();
    }

    template<typename Handler>
    inline
    Scheduler<Handler>::Scheduler(Handler&& handler, long millis)
    : handler_(std::move(handler))
    , millis_(millis > 0 ? millis : 0)
    , runner_(&Scheduler::run_, this)
    {}

    template<typename Handler>
    inline void
    Scheduler<Handler>::stop()
    {
        Lock    _lock(mx_);
        if ( stopped_ ) { return; }
        stopped_ = true;
        _lock.unlock();
        ready_.notify_all();
        runner_.join();
    }

    template<typename Handler>
    inline std::size_t
    Scheduler<Handler>::size() //const
    {
        Guard   _guard(mx_);
        return map_.size();
    }

    template<typename Handler>
    inline TimePoint
    Scheduler<Handler>::schedule( Item& item )
    {
        return schedule( item, millis_ );
    }

    template<typename Handler>
    inline TimePoint
    Scheduler<Handler>::schedule( Item& item, long millis )
    {
        if ( millis <= 0 ) { return SysClock::now(); } // don't schedule
        Guard   _guard(mx_);
        auto    _ptr(map_.insert( {SysClock::now() + MilliSecs(millis), item} ));
        if ( _ptr == map_.begin() ) { ready_.notify_one(); }
        return _ptr->first;
    }

    template<typename Handler>
    inline std::size_t
    Scheduler<Handler>::cancel( TimePoint tp, Item& item )
    {
        Guard   _guard(mx_);
        list_.emplace_back( tp, item );
        ready_.notify_one();
        return map_.size();
    }

    template<typename Handler>
    inline bool
    Scheduler<Handler>::is_active_( Value const& val ) const
    {
        return std::find( list_.begin(), list_.end(), val ) == list_.end(); // not in list_
    }

    //!> run: map_ is automatically sorted by TimePoint,
    //!> thus map_.begin() is the earliest future timeout point.
    template<typename Handler>
    inline void
    Scheduler<Handler>::run_()
    {
        Lock    _lock(mx_);
        while ( !stopped_ )
        {
            if ( !list_.empty() ) { purge_(); }
            if ( !map_.empty() ) // busy
            {
                auto    _ptr(map_.begin());
                if ( ready_.wait_until( _lock, _ptr->first ) == std::cv_status::timeout
                  && is_active_( *_ptr ) )
                {
                    _lock.unlock();
                    handler_( _ptr->second );
                    _lock.lock();
                    map_.erase( _ptr );
                }
            }
            else { ready_.wait( _lock ); } // idle
        }
    }

    template<typename Handler>
    inline void
    Scheduler<Handler>::purge_() // under lock!
    {
        for ( auto& _item : list_ )
        {
            // search for TimePoint
            auto    _pr(map_.equal_range( _item.first ));
            // search for Item
            while ( _pr.first != _pr.second )
            {
                if ( _item.second == _pr.first->second )
                {
                    map_.erase( _pr.first );
                    break;
                }
                ++_pr.first;
            }
        }
        list_.clear();
    }

} // namespace Utility

#endif // UTILITY_SCHEDULER_H
