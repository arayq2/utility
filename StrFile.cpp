
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
        if ( fsize_ % pageSize ) { map_normal_( _fd, fsize_ ); }
        else { map_special_( _fd, fsize_ ); }

        ::close( _fd );
    }

    void
    StrFile::map_normal_( int fd, std::size_t size )
    {
        msize_ = size + pageSize - (size % pageSize);
        base_ = ::mmap( nullptr, msize_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
        if ( base_ == MAP_FAILED )
        {
            err_ = errno;
            base_ = nullptr;
        }
    }
    
    void
    StrFile::map_special_( int fd, std::size_t size )
    {
        msize_ = size + pageSize;

        auto    _base(::mmap( nullptr, msize_, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ));
        if ( _base == MAP_FAILED )
        {
            err_ = errno;
            return;
        }
        base_ = ::mmap( _base, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0 );
        if ( base_ == MAP_FAILED )
        {
            err_ = errno;
            base_ = nullptr;
			return;
        }
#define ADJUST_(V,S) static_cast<void*>(static_cast<char *>(V) + S)
		_base = ADJUST_(_base, size);
#undef ADJUST_
        ::mmap( _base, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0 );
        if ( _base == MAP_FAILED )
        {
            err_ = errno;
        }
    }
} // namespace Utility
