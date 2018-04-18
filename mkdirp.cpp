
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <iostream>

namespace FileSys
{
    using strc = std::string const;

    bool check_err( int err, int ok_err, char const* ok_err_str )
    {
        if ( err == ok_err ) { return true; }
        std::cerr << "errno (!=" << ok_err_str << "): " << ::strerror( err ) << std::endl;
        return false;
    }
    
    bool check_mode( strc& dir,  mode_t mode )
    {
        if ( S_ISDIR( mode ) ) { return true; }
        std::cerr << "File [" << dir << "]is not a directory." << std::endl;
        return false;
    }
    
    bool is_dir( strc& dir )
    {
        struct stat     _buf;
        return ::stat( dir.c_str(), &_buf ) == 0
        ? check_mode( dir, _buf.st_mode )
        : false
        ;
    }
    
    bool make_dir( strc& dir, mode_t mode )
    {
        errno = 0;
        return ::mkdir( dir.c_str(), mode ) == 0
        ? true // success
        : check_err( errno, EEXIST, "EEXIST" )
        ? is_dir( dir )
        : false
        ;
    }
    
    bool make_path( strc&, mode_t );
    bool check_path( strc& dir, mode_t mode )
    {
        errno = 0;
        return is_dir( dir )
        ? true 
        : errno != 0 and check_err( errno, ENOENT, "ENOENT" )
        ? make_path( dir, mode )
        : false
        ;
    }
    
    bool make_path( strc& dir, mode_t mode )
    {
        size_t  _pos(dir.find_last_of( '/' ));
        return _pos == 0 or _pos == std::string::npos or check_path( dir.substr( 0, _pos ), mode )
        ? make_dir( dir, mode )
        : false
        ;
    }
} // namespace FileSys

int main( int ac, char *av[] )
{
    if ( ac < 2 ) { std::cerr << "No path!" << std::endl; return 1; }
    std::cerr << "path [" << av[1] << "] " 
        << (FileSys::make_path( av[1], 01777 ) ? "succeeded!" : "failed!") << std::endl;
    return 0;
}
