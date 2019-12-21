#pragma once


#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

namespace System
{
    long 
    get_tid() { return ::syscall( SYS_gettid ); }

} // namespace System
