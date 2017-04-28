#pragma once


#include <ostream>

    template<bool> char const* alpha_( bool );
    
    template<>
    char const* alpha_<true>( bool value )
    {
        return value ? "TRUE" : "FALSE";
    }   
    
    template<>
    char const* alpha_<false>( bool value )
    {
        return value ? "true" : "false";
    }   

    template<bool UPPER = false>
    class BoolAlpha
    {
    public:
        BoolAlpha(bool value) : value_(alpha_<UPPER>(value)) {}
        
        operator std::string () const { return value_; }
        
        friend
        std::ostream& operator<<( std::ostream& os, BoolAlpha<UPPER> b )
        {
            return b.output_( os );
        }
        
    private:
        char const*    value_;

        std::ostream& output_( std::ostream& os ) const
        {
            return os << value_;
        }
        
    };