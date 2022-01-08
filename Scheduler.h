/* =====================================================================
 + Copyright 2019-2022 Arjun Ray
 + Released under MIT License (see https://mit-license.org
 + ===================================================================== */
#pragma once

#ifndef UTILITY_SCHEDULER_H
#define UTILITY_SCHEDULER_H

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

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
        using Map       = std::multimap<TimePoint, Item>;
        using List      = std::vector<typename Map::value_type>;
        using Mutex     = std::mutex;
        using Guard     = std::lock_guard<Mutex>;
        using Lock      = std::unique_lock<Mutex>;
        using Condition = std::condition_variable;
        using Thread    = std::thread;
    public:

        ~Scheduler() noexcept;
        explicit Scheduler(Handler&& handler, long millis = 0);

        void stop();
        std::size_t size() const;
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
            ~Watch() noexcept { if ( !keep_ ) { scheduler_.cancel( tp_, item_ ); } }
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

            bool keep( bool val ) { return (keep_ = val); }

        private:
            Scheduler&  scheduler_;
            Item&       item_;
            TimePoint   tp_;
            bool        keep_{false};

            Watch(Watch const&) = delete;
            Watch& operator=( Watch const& ) = delete;
        };

    private:
        bool        stopped_{false};
        Mutex       mx_;
        Condition   ready_;
        Map         map_;
        List        list_;
        Handler     handler_;
        long        millis_;
        Thread      runner_;

        void run_();
        void purge_(); // under lock!
    };

    template<typename Handler>
    inline
    Scheduler<Handler>::~Scheduler() noexcept { stop(); }

    template<typename Handler>
    inline
    Scheduler<Handler>::Scheduler(Handler&& handler, long millis) 
    : handler_(std::move(handler))
    , millis_( millis > 0 ? millis : 0)
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
    Scheduler<Handler>::size() const
    {
        Guard   _guard(mx_);
        return map_.size();
    }

    template<typename Handler>
    inline TimePoint
    Scheduler<Handler>::schedule( Item& item ) { return schedule( item, millis_ ); }

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
    }

    //!> run: map_ is automatically sorted by Timepoint,
    //!> thus map_.begin() is the earliest future timeout.
    template<typename Handler>
    inline void
    Scheduler<Handler>::run_()
    {
        Lock    _lock(mx_);
        while ( !stopped_ )
        {
            if ( !list_.empty() ) { purge_(); }
            if ( !map_.empty() ) //busy
			{
                auto    _ptr(map_.begin());
                if ( ready_.wait_until( _lock, _ptr->first ) == std::cv_status::timeout )
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
    Scheduler<Handler>::purge_() // under lock
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
