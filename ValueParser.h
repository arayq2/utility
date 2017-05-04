#pragma once

#include <string>
#include <iterator>
#include <cstring>  // ::strlen

namespace Utility
{

    template<typename ValueType>
    struct ValueParserMethods
#ifdef PREFER_LINKTIME_ERRORS
    {
        static inline
        ValueType extract( char const* start, char*& next, char const delim );
        
        static inline
        void adjust( char const*& start, char*& next, char const delim );
    }
#else
    // compile-time error if specialization not found
#endif
    ;
    
    template<typename ValueType, typename Methods = ValueParserMethods<ValueType> >
    class ValueParser
    {
    public:
        ValueParser(std::string const& source, char const* delim = ",")
        : ValueParser(source.c_str(), source.length(), delim)
        {}
        
        ValueParser(char const* source, char const* delim)
        : ValueParser(source, ::strlen( source ), delim) 
        {}
        
        ValueParser(char const* source, size_t length, char const* delim)
        : source_(source)
        , length_(length)
        , delim_(delim[0])
        {}
    
        using value_type = ValueType;
        
        class iterator
        : public std::iterator<std::input_iterator_tag, value_type>
        {
        public:
            iterator(char const* start, char const delim)
            : start_(start)
            , next_(const_cast<char*>(start_))
            , delim_(delim)
            {}
            
            iterator& operator->() { return *this; }

            value_type operator*() const 
            {
                return Methods::extract( start_, next_, delim_ );
            }
            
            iterator& operator++()
            {
                Methods::adjust( start_, next_, delim_ );
                return *this;
            }
            
            bool operator!=( iterator const& rhs ) const { return start_ != rhs.start_; }
            bool operator==( iterator const& rhs ) const { return start_ == rhs.start_; }
       
        private:
            char const*     start_;
            mutable char*   next_;
            char const      delim_;
        };

        iterator begin() { return iterator(source_, delim_); }
        iterator end()   { return iterator(source_ + length_, delim_); }
        
    private:
        char const* source_;
        size_t      length_;
        char const  delim_;
    };
} // namespace Utility

#include <cstdlib>
#include <limits>

namespace Utility
{  
    template<>
    struct ValueParserMethods<double>
    {
        static inline
        double extract( char const* start, char*& next, char const )
        {
            return *start
            ? ::strtod( start, &next )
            : std::numeric_limits<double>::quiet_NaN()
            ; 
        }
        
        static inline
        void adjust( char const*& start, char*& next, char const delim )
        {
            while ( *next and *next != delim ) { ++next; }
            start = *next ? ++next : next;
        }
    };
    
    using DoubleParser = ValueParser<double>;
    
} // namespace Utility
