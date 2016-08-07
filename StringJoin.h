#pragma once

#include <ostream>

namespace Utility
{
    template<typename Iterator>
    class StringJoin
    {
    public:
        StringJoin(Iterator begin, Iterator const end, char const* delim = ",")
        : begin_(begin)
        , end_(end)
        , delim_(delim)
        {}
        
        char const* delim( char const* value )
        {
            char const*     _rv(delim_);
            
            delim_ = value;
            return _rv;
        }
        
        template<typename Output>
        Output& output( Output& os ) const
        {  
            Iterator    _current(begin_);
            
            if ( _current != end_ )
            {
                os << *_current;
                while ( ++_current != end_ )
                {
                    os << delim_ << *_current;
                }
            }
            return os;
        }
        
    private:
        Iterator        begin_;
        Iterator const  end_;
        char const*     delim_;
    };
    
} // namespace Utility

template<typename Iterator>
std::ostream& operator<< ( std::ostream& os, Utility::StringJoin<Iterator> const& joiner )
{
    return joiner.output( os );
}