#pragma once

#ifndef UTILITY_EXECUTOR_H
#define UTILITY_EXECUTOR_H

#include <vector>
#include <cstring>
#include <sstream>
#include <unistd.h>

namespace Utility
{
    template<typename LogPolicy>
    class Executor
    {
    public:
        ~Executor() noexcept = default;
        Executor() = default;
        template<typename Iterator>
        Executor(Iterator start, Iterator const end, bool noinsert = false);

        void operator()( std::string const& path = std::string() );
        
        void dump();
    private:
        std::vector<char*>  argv_;
        std::size_t         interp_{0};
        bool                insert_{false};
        bool                active_{false};
    };

    template<typename LogPolicy>
    template<typename Iterator>
    Executor<LogPolicy>::Executor(Iterator start, Iterator const end, bool noinsert)
    : argv_(start, end)
    , interp_(0)
    , insert_(!noinsert)
    , active_(argv_.size() > 0)
    {
        // find placeholder slot
        for ( std::size_t _i{0}; _i < argv_.size(); ++_i )
        {
            if ( 0 == ::strcmp( argv_[_i] = "{}" ) )
            {
                interp_ = _i;
                break;
            }
        }
        // no slot found, make space at end unless excluded
        if ( interp_ == 0 && insert_ )
        {
            interp_ = argv_.size();
            argv_.push_back( nullptr );
        }
        argv_.push_back( nullptr ); // for execv
        dump();
    }

    template<typename LogPolicy>
    void Executor<LogPolicy>::operator()( std::string const& path )
    {
        if ( !active_ )
        {
            LogPolicy().on_error( "Executor: no command to execute." );
            return;
        }
        if ( insert_ )
        {
            argv_[interp_] = const_cast<char*>(path.c_str());
        }
        if ( 0 == ::fork() )
        {   // in child process
            ::setsid();
            ::execv( argv_[0], &argv_[0] );
            LogPolicy().on_error( std::string("execv failed") + ::strerror( errno ) );
            ::exit( 127 );
        }
        if ( insert_ )
        {
            argv_[interp_] = nullptr;
        }
    }

    template<typename LogPolicy>
    void Executor<LogPolicy>::dump()
    {
        std::ostringstream      _oss;
        _oss << "argv has size [" << argv_.size() << "] with interp = [" << interp_ << "]: ";
        for ( auto const& _ptr : argv_ ) { _oss << "[" << (_ptr ? _ptr : "NULL") << "]"; }
        LogPolicy().on_info( _oss.str() );
    }


} // namespace Utility

#endif // UTILITY_EXECUTOR_H
