#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

#include <cxxabi.h>
#include <execinfo.h>

namespace Utility
{   
    // CallStack.
    // Aggregates GNU compiler callstack.
    //
    class CallStack
    {
        enum { ST_SIZE = 100 }; // deeper callstacks are in trouble anyway
    public:
        CallStack()
        : offset_(1)
        , depth_(::backtrace( stack_, ST_SIZE ))
        , symbols_(::backtrace_symbols( stack_, depth_ ))
        {}
        
        template<typename Action>
        CallStack(Action&& action)
        : CallStack()
        {
            offset_ = 2;
            for_each( action );
        }
        
        template<typename Action>
        CallStack(Action const& action)
        : CallStack()
        {
            offset_ = 2;
            for_each( action );
        }
        
        ~CallStack() { ::free( symbols_ ); }
        
        using iterator = char**;
        
        iterator begin() const { return symbols_ + offset_; }
        iterator end()   const { return symbols_ + depth_;  }
        
        template<typename Action>
        void for_each( Action const& action ) const
        {
            for ( auto const& _symbol : *this ) { action( _symbol ); }
        }
        
    private:
        size_t  offset_; // ignore our ctor(s) at top of call stack
        int     depth_;
        char**  symbols_;
        void*   stack_[ST_SIZE];
    };
    
    // Demangler.
    // Tries to demangle a C++ ABI name.
    //
    class Demangler
    {
    public:
        Demangler(std::string const& symbol)
        : buffer_(abi::__cxa_demangle( symbol.c_str(), nullptr, nullptr, &status_ ))
        {
            switch ( status_ )
            {
            case  0 : str_ = std::string(buffer_); break;
            case -1 : str_.assign( "(Memory allocation failure!)" ); break;
            case -2 : str_.assign( symbol ); break; // invalid mangled name
            default : str_.assign( "(Invalid argument in call!)" ); break;
            }
        }
        
        Demangler(char const* begin, char const* end)
        : Demangler(std::string(begin, end))
        {}
        
        ~Demangler() { ::free( buffer_ ); }
        
        operator std::string const& () const { return str_; }
        
    private:
        int         status_{0};
        char*       buffer_;
        std::string str_;
    };

    // SymbolAnalyser.
    // Tries to determine whether a callstack entry has a name symbol.
    //
    class SymbolAnalyser
    {
    public:
        SymbolAnalyser(char* symbol)
        : begin_(::strchr( symbol, '(' ))
        , end_(::strchr( symbol, '+' ))
        , str_(symbol) // default
        {
            if ( begin_ and end_ ) { str_ = Demangler(begin_ + 1, end_); }
        }
        
        operator std::string const& () const { return str_; }
        
    private:
        char*       begin_;
        char*       end_;
        std::string str_;
    };

} // namespace Utility

