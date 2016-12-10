#pragma once

#include <ostream>

namespace Utility
{
    // SequenceJoin.
    // Insert a sequence into a stream with a delimiter,
    // and optionally a transformation
    //
    template<typename Iterator, typename Transform = void>
    class SequenceJoin
    {
    public:
        SequenceJoin(Iterator begin, Iterator const end, char const* delim = ",")
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
                os << Transform(*_current);
                while ( ++_current != end_ )
                {
                    os << delim_ << Transform(*_current);
                }
            }
            return os;
        }
        
        friend
        std::ostream& operator<< ( std::ostream& os, Utility::SequenceJoin<Iterator, Transform> const& joiner )
        {
            return joiner.output( os );
        }

    private:
        Iterator        begin_;
        Iterator const  end_;
        char const*     delim_;
    };
 
 
    // no transform case by specialization
    //
    template<typename Iterator>
    class SequenceJoin<Iterator, void>
    {
    public:
        SequenceJoin(Iterator begin, Iterator const end, char const* delim = ",")
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
        
        friend
        std::ostream& operator<< ( std::ostream& os, Utility::SequenceJoin<Iterator, void> const& joiner )
        {
            return joiner.output( os );
        }

    private:
        Iterator        begin_;
        Iterator const  end_;
        char const*     delim_;
    };
 
} // namespace Utility

