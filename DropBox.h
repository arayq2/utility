#pragma once

#ifndef UTILITY_DROPBOX_H
#define UTILITY_DROPBOX_H

#include <map>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace Utility
{
    namespace chr = std::chrono;

    /**
     * @class DropBox
     * @brief Inter-thread resource transfer with timeout.
     * std::promise-std::future pairs do not support timeout.
     * Associated race conditions are avoided by determining
     * responsibility for disposal of DropBox (and resource).
     *
     * Mutex is shared across multiple active DropBox instances for economy.
     */
    template <typename Payload>
    class DropBox
    {
        using Mutex     = std::mutex;
        using Guard     = std::lock_guard<Mutex>;
        using Lock      = std::unique_lock<Mutex>;
        using Condition = std::condition_variable;
        using TimePoint = chr::time_point<chr::system_clock>;
    public:
        ~DropBox() noexcept = default;
        DropBox(Mutex* mp)
        : mp_(mp)
        , payload_()
        {}

        //!> returns to producer: whether consumer has abandoned waiting.
        bool transfer( Payload&& pl )
        {
            Guard   _guard(*mp_);
            if ( !canceled_ )
            {
                payload_  = std::move( pl );
                supplied_ = true;
                ready_.notify_all();
            }
            return canceled_;
        }

        //!> returns to consumer: whether producer has supplied resource.
        bool wait_for( long millis )
        {
            TimePoint   _limit(chr::system_clock::now() + chr::milliseconds(millis));
            Lock        _lock(*mp_);
            while ( !supplied_ && std::cv_status::timeout != ready_.wait_until( _lock, _limit ) )
            {}
            canceled_ = true;
            return supplied_;
        }

        Payload& get_payload() const
        {
            Guard   _guard(*mp_);
            return payload_;
        }

    private:
        Mutex*      mp_;
        Condition   ready_;
        Payload     payload_;
        bool        supplied_{false};
        bool        canceled_{false};
    };

    /**
     * @class DropBank
     * @brief Aggregates active DropBox instances.
     * Manages acquisition of DropBox instances.
     */
    template <typename Key, typename Payload>
    class DropBank
    {
        using Mutex     = std::mutex;
        using Guard     = std::lock_guard<Mutex>;
        using TimePoint = chr::time_point<chr::system_clock>;
    public:
        using Box = DropBox<Payload>;
        using Map = std::map<Key const, std::pair<TimePoint, Box>>;

        ~DropBank() noexcept = default;
        DropBank() = default;

        //!> called by consumer.
        Box* acquire( Key const& key )
        {
            Guard   _guard(mx_);
            auto    _itr(map_.emplace( std::make_pair( key, {chr::system_clock::now(), &mx_} ) ));
            return &_itr.first->second.second;
        }

        //!> called by producer.
        Box* retrieve( Key const& key )
        {
            Guard   _guard(mx_);
            auto    _itr(map_.find( key ));
            return _itr == map_.end() ? nullptr : _itr->second.second;
        }

        //!> called by final resource holder.
        void dispose( Key const& key )
        {
            Guard   _guard(mx_);
            auto    _itr(map_.find( key ));
            if ( _itr != map_.end() ) { map_.erase( _itr ); }
        }

        void purge( long threshold )
        {
            TimePoint   _limit(chr::system_clock::now() - chr::milliseconds(threshold));
            Guard       _guard(mx_);
            auto        _itr(map_.begin());
            while ( _itr != map_.end() )
            {
                if ( _itr->second.first < _limit ) { _itr = map_.erase( _itr ); }
                else { ++_itr; }
            }
        }

    private:
        Mutex   mx_;
        Map     map_;
    };

} // namespace Utility

#endif // UTILITY_DROPBOX_H
