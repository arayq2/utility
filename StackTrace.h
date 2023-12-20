/** ======================================================================+
 + Copyright @2020-2021 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#include "CallStack.h"
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace Utility
{   
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
    //      (CallstackPrinter(std::cerr))( CallStack() );
    // Or:
    //      CallStack(CallstackPrinter(std::cerr));
    // Yes, both ways work!:-)
    //
    class CallstackPrinter
    {
    public:
        CallstackPrinter(std::ostream& os)
        : printer_(os)
        {}
        
        // could use a lambda instead
        void operator() ( CallStack&& stack ) const { stack.for_each( *this ); }
        void operator() ( CallStack const& stack ) const { stack.for_each( *this ); }
        
        void operator() ( char* symbol ) const { printer_( SymbolAnalyser(symbol) ); }
        
    private:
        SymbolPrinter   printer_;
    };
    
    // StackTrace.
    // Holds a stack trace as a string.
    //
    class StackTrace
    {
    public:
        explicit
        StackTrace(char const* prefix = "Stack trace:\n")
        : oss_(prefix, std::ostringstream::ate)
        , printer_(oss_)
        , stack_(printer_, 2)
        {}
        
        std::string const get() const { return oss_.str(); }
        
        operator std::string const () const { return get(); }

    private:
        std::ostringstream  oss_;
        CallstackPrinter    printer_;
        CallStack           stack_;
    };

    // TrapException.
    // Capture stack trace as part of conversion of
    // hardware trap (synchronous signal) to exception.
    //
    struct TrapException
    : public std::runtime_error
    {
        explicit
        TrapException(char const* prefix)
        : std::runtime_error(StackTrace(prefix))
        {}
    };
    
} // namespace Utility

