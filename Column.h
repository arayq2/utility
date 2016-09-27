#pragma once

#include <vector>
#include <iterator>
#include <stdexcept>    
    
namespace Utility
{
    template<typename T>
    T* throwIfNull( T* ptr )
    {
        if ( ptr ) { return ptr; }
        throw std::runtime_error("Null pointer initialization");
    }

    // Column.
    // A vector-like interface to a sequence of field values 
    // ("column") in a vector of structs ("rows"). 
    // This is an alternative to a "parallel arrays" strategy  
    // of organizing a matrix of values, such as a collection  
    // of time series with cross-sectional inter-relationships.
    //
    template<typename T, typename Row>
    class Column
    {
    public:
        using Matrix = std::vector<Row>;
        using Field  = T Row::*;
    
        Column(Matrix* matrix, Field field)
        : matrix_(throwIfNull(matrix))
        , field_(field)
        {}
        
        using value_type = T;
        
        value_type&       operator[] ( size_t index )       { return matrix_->at( index ).*field_; }
        value_type const& operator[] ( size_t index ) const { return matrix_->at( index ).*field_; }
        
        size_t size() const { return matrix_->size(); }
        
    // iterator support
        class iterator
        : public std::iterator<std::forward_iterator_tag, value_type>
        {
        public:
            iterator(Column* column, size_t index = 0)
            : column_(throwIfNull(column))
            , index_(index)
            {}
            
            iterator& operator++ ()    { ++index_; return *this; }
            iterator  operator++ (int) { ++index_; return iterator(column_, index_ - 1); }
            
            value_type&       operator* ()       { return (*column_)[index_]; }
            value_type const& operator* () const { return (*column_)[index_]; }
            
            bool operator!= ( iterator const& rhs ) const { return index_ != rhs.index_; }
            bool operator== ( iterator const& rhs ) const { return index_ == rhs.index_; }
            bool operator<  ( iterator const& rhs ) const { return index_ <  rhs.index_; }

        private:
            Column*     column_;
            size_t      index_;
        };
        
        iterator begin() { return iterator(this); }
        iterator end()   { return iterator(this, size()); }
   
    private:
        Matrix*     matrix_;
        Field       field_;
    };
    
} // namespace Utility

#if 0
#endif