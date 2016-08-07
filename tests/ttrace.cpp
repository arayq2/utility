
#include "StackTrace.h"

#include <iostream>

    // Adapted from manpage for ::backtrace

    void
    myfunc3()
    {
        using namespace Utility;
        (CallstackPrinter(std::cerr))( StackTrace() );
        std::cerr << "====[one more time!]====" << std::endl;
        StackTrace(CallstackPrinter(std::cerr));
    }

    static void   /* "static" means don't export the symbol... */
    myfunc2()
    {
        myfunc3();
    }

    void
    myfunc( int ncalls )
    {
        if ( ncalls > 1 )
            myfunc( ncalls - 1 );
        else
            myfunc2();
    }

    int
    main( int argc, char *argv[] )
    {
        if ( argc != 2 )
        {
            std::cerr << "Usage: " << argv[0] << " num-calls" << std::endl;
            return 1;
        }

        myfunc( ::atoi( argv[1] ) );
        return 0;
    }
