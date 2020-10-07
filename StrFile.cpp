
#include "StrFile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

namespace Utility
{
namespace
{
    long    pageSize{::sysconf( _SC_PAGE_SIZE )};
}

    StrFile::~StrFile() noexcept
    {
        if ( base_ ) { ::munmap( base_, msize_ ); }
    }

    StrFile::StrFile(char const* file)
    {
        int     _fd(::open( file, O_RDONLY ));
        if ( _fd < 0 )
        {
            err_ = errno;
            return;
        }

        struct stat _stb;
        if ( fstat( _fd, &_stb ) < 0 )
        {
            err_ = errno;
            ::close( _fd );
            return;
        }

        fsize_ = _stb.st_size;
        msize_ = fsize_ + pageSize - (fsize_ % pageSize);

        auto    _base(::mmap( nullptr, msize_, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ));
        if ( _base == MAP_FAILED )
        {
            err_ = errno;
            ::close( _fd );
            return;
        }
        base_ = ::mmap( _base, fsize_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, _fd, 0 );
        if ( _base == MAP_FAILED )
        {
            err_ = errno;
            base_ = nullptr;
        }

        ::close( _fd );
    }
} // namespace Utility
