#pragma once

#include <ostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include <cxxabi.h>
#include <execinfo.h>

namespace Utility
{   
    // StackTrace.
    // Aggregates GNU compiler callstack.
    //
    class StackTrace
    {
        enum { ST_SIZE = 100 }; // deeper callstacks are in trouble anyway
    public:
        StackTrace()
        : offset_(1)
        , depth_(::backtrace( stack_, ST_SIZE ))
        , symbols_(::backtrace_symbols( stack_, depth_ ))
        {}
        
        template<typename Action>
        StackTrace(Action&& action)
        : StackTrace()
        {
            offset_ = 2;
            apply( action );
        }
        
        template<typename Action>
        StackTrace(Action const& action)
        : StackTrace()
        {
            offset_ = 2;
            apply( action );
        }
        
        ~StackTrace() { ::free( symbols_ ); }
        
        template<typename Action>
        void apply( Action const& action ) const
        {
            std::for_each( symbols_ + offset_, symbols_ + depth_, action );
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

    // SymbolAnalyzer.
    // Tries to determine whether a callstack entry has a name symbol.
    //
    class SymbolAnalyzer
    {
    public:
        SymbolAnalyzer(char* symbol)
        : begin_(::strchr( symbol, '(' ))
        , end_(::strchr( symbol, '+' ))
        , str_(symbol)
        {
            if ( begin_ and end_ ) { str_ = Demangler(begin_ + 1, end_); }
        }
        
        operator std::string const& () const { return str_; }
        
    private:
        char*       begin_;
        char*       end_;
        std::string str_;
    };
    
    // SYmbolPrinter.
    // Pretty-prints callstack entries.
    //
    class SymbolPrinter
    {
    public:
        SymbolPrinter(std::ostream& os)
        : os_(os)
        , slot_(0)
        {}
        
        void operator() ( std::string const& symbol ) const
        {
            os_ << (slot_ < 10 ? " [" : "[") << slot_ << "] " << symbol << "\n";
            ++slot_;
        }

    private:
        std::ostream&   os_;
        int mutable     slot_;
    };
    
    // CallstackPrinter.
    // Organizes pretty-printing of a GNU compiler callstack.
    //
    // Sample usage:
    //      (CallstackPrinter(std::cerr))( StackTrace() );
    // Or:
    //      StackTrace(CallstackPrinter(std::cerr));
    // Yes, both ways work!:-)
    //
    class CallstackPrinter
    {
    public:
        CallstackPrinter(std::ostream& os)
        : printer_(os)
        {}
        
        // could use a lambda instead
        void operator() ( StackTrace&& trace ) const { trace.apply( *this ); }
        void operator() ( StackTrace const& trace ) const { trace.apply( *this ); }
        
        void operator() ( char* symbol ) const { printer_( SymbolAnalyzer(symbol) ); }
        
    private:
        SymbolPrinter   printer_;
    };

} // namespace Utility

