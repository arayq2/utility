#pragma once

#include <ostream>

namespace Utility
{
    struct LocationFormat
    {
        static std::ostream& output( std::ostream& os, char const* function, char const* file, int line )
        {
            return line
            ? (os << function_ << "@" << file << ":" << line)
            : (os << "(No location information)")
            ;            
        }
    };

    template<typename Format = LocationFormat>
    class Location
    {
    public:
        Location(char const* function, char const* file, int line)
        : function_(function)
        , file_(file)
        , line_(line)
        {}

        Location()
        : function_("")
        , file_("")
        , line_(0)
        {}

        std::ostream& output( std::ostream& os ) const
        {
            return Format::output( os, function_, file_, line_ );
        }

        friend inline
        std::ostream& operator<<( std::ostream& os, Location const& location )
        {
            return location.output( os );
        }

    private:
        char const*     function_;
        char const*     file_;
        int             line_;
    };

} // namespace Utility
