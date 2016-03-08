#pragma once

#include <ostream>

namespace Utility
{
    class Location
    {
    public:
        Location(char const* function, char const* file, int line)
        : function_(function)
        , file_(file)
        , line_(line)
        {}
        
        std::ostream& output( std::ostream& os ) const
        {
            return line_
            ? (os << function_ << "(" << file_ << ":" << line_ << ")")
            : (os << "(No location information)")
            ;
        }
        
    private:
        char const*     function_;
        char const*     file_;
        int             line_;
    };
    
    inline
    std::ostream& operator<< ( std::ostream& os, Location const& location )
    {
        return location.output( os );
    }

} // namespace Utility
