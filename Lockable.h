#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>
#include <thread>

namespace Utility
{
    class Lockable
    {
    public:
        Lockable() : mutex_() {}
        
        void lock()     const { mutex_.lock(); }
        void unlock()   const { mutex_.unlock(); }
        bool try_lock() const { return mutex_.try_lock(); }
        
        // needed for condvars.  This new interface sucks.
        operator std::mutex& () const { return mutex_; }
        
        class CondVar
        {
        public:
        
            void signal()    { cv_.notify_one(); }
            void broadcast() { cv_.notify_all(); }
            
            void wait( std::unique_lock<std::mutex>& lock )
            {
                cv_.wait( lock );
            }
        
        private:
            std::condition_variable     cv_;
        };
        
    private:
        // actual locking is an implementation detail
        std::mutex mutable  mutex_;
        
        Lockable(Lockable const&) = delete;
        Lockable(Lockable&&) = delete;
        Lockable& operator= ( Lockable const& ) = delete;
    };
    
    using Condition = Lockable::CondVar;
    
    class AutoLock
    {
        Lockable const*     lp_;
    public:
        AutoLock(Lockable const* lp) : lp_(lp) { lp_->lock(); }
        ~AutoLock() { lp_->unlock(); }
    };

    class WithLock
    {
        Lockable const*     lp_;
    public:
        WithLock(Lockable const* lp) : lp_(lp) {}
        
        template<typename Action>
        void operator() ( Action&& action )
        {
            AutoLock    _lock(lp_);
            action();
        }
    };

#define AUTOLOCK()      Utility::AutoLock           _guard_(this)
#define THISLOCK(X)     std::unique_lock<std::mutex>    X(*this)
#define WITHLOCK()      Utility::WithLock(this)
    
    class Relockable
    {
    public:
        Relockable() : mutex_() {}
   
        void lock()     const { mutex_.lock(); }
        void unlock()   const { mutex_.unlock(); }
        bool try_lock() const { return mutex_.try_lock(); }
        
        // needed for condvars.  This new interface sucks.
        operator std::recursive_mutex& () const { return mutex_; }
                
    private:
        // actual locking is an implementation detail
        std::recursive_mutex mutable    mutex_;
        
        Relockable(Relockable const&) = delete;
        Relockable(Relockable&&) = delete;
        Relockable& operator= ( Relockable const& ) = delete;
    };

    class AutoRelock
    {
        Relockable const*       lp_;
    public:
        AutoRelock(Relockable const* lp) : lp_(lp) { lp_->lock(); }
        ~AutoRelock() { lp_->unlock(); }
    };

#define AUTORELOCK()    Utility::AutoRelock         _guard_(this)

    class Event
    : private Lockable
    {
    public:
        explicit
        Event(size_t count = 1)
        : condition_()
        , count_(count)
        {}
        
        void wait()
        {
            THISLOCK(_lock);
            while ( count_ > 0 ) { condition_.wait( _lock ); }
        }
        
        size_t signal()
        {
            AUTOLOCK();
            if ( count_ > 0 )
            {
                --count_;
                if ( count_ == 0 ) { condition_.broadcast(); }
            }
            return count_;
        }
        
        size_t post( size_t add = 0 )
        {
            AUTOLOCK();
            count_ += add;
            return count_;
        }

        size_t cancel( size_t sub )
        {
            AUTOLOCK();
            if ( count_ < sub ) { count_ = 0; }
            else                { count_ -= sub; }
			if ( count_ == 0 ) { condition_.broadcast(); }
            return count_;
        }

		void stop()
		{
			AUTOLOCK();
			count_ = 0;
			condition_.broadcast();
		}
    private:
        Condition   condition_;
        size_t      count_;
    };

} // namespace Utility
