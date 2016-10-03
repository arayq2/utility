#pragma once

#include <vector>
#include <string>
#include <iterator>
#include <stdexcept>    

#include "StringKeyMap.h"
    
namespace Utility
{
    template<typename T>
    T* throwIfNull( T* ptr, char const* errmsg )
    {
        if ( ptr ) { return ptr; }
        throw std::runtime_error(errmsg);
    }

    template<typename, bool> struct NoCheckPolicy;
    
    template<typename T>
    struct NoCheckPolicy<T, false>
    {
        static T&       at( std::vector<T>& vec, size_t index )       { return vec.at( index ); }
        static T const& at( std::vector<T> const& vec, size_t index ) { return vec.at( index ); }
    };
    
    template<typename T>
    struct NoCheckPolicy<T, true>
    {
        static T&       at( std::vector<T>& vec, size_t index )       { return vec[index]; }
        static T const& at( std::vector<T> const& vec, size_t index ) { return vec[index]; }
    };

    // Column.
    // A vector-like interface to a sequence of field values ("column") in 
    // a vector of records ("rows"). 
    // This is an alternative to a record of (equal-sized) vectors (or the 
    // "parallel arrays" strategy) to organize a matrix of values, such as
    // a collection of time series with cross-sectional inter-relationships.
    //
    template<typename T, typename Row, bool NoCheck = false>
    class Column
    {
    public:
        using Matrix = std::vector<Row>;
        using Field  = T Row::*;
        using Method = NoCheckPolicy<Row, NoCheck>;
    
        Column(Matrix* matrix, Field field)
        : matrix_(throwIfNull( matrix, "Null Matrix* initialization" ))
        , field_(field)
        {}
        
        using value_type = T;
        
        value_type&       operator[] ( size_t index )       { return Method::at( *matrix_, index ).*field_; }
        value_type const& operator[] ( size_t index ) const { return Method::at( *matrix_, index ).*field_; }
        
        size_t size() const { return matrix_->size(); }
        
    // iterator support
        class iterator
        : public std::iterator<std::forward_iterator_tag, value_type>
        {
        public:
            iterator(Column* column, size_t index = 0)
            : column_(throwIfNull( column, "Null Column* initialization" ))
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
    
    // ColumnAccess.
    // Helper class to simplify the bolierplate of using the Column 
    // template.
    // The Client class should
    //  1. Derive publicly from ColAccess<Client>, to inherit the 
    //     XXXColumn() methods.
    //  2. Declare the class ColumnAccess<Client> a friend, to give 
    //     it access to fields.
    //  3. Populate the field maps (intMap_, dblMap_, strMap_).
    //
    // Note that the boilerplate can be reproduced explicitly for 
    // other member types, with only the Column creation methods 
    // visible publicly. See also: col_example.cpp
    //
    template<typename Client>
    class ColumnAccess
    {
    public:
        using rows   = std::vector<Client>;

        template<typename T, bool NoCheck = false>
        using column = Column<T, Client, NoCheck>;
        
        template<bool NoCheck = false>
        static column<int, NoCheck> intColumn( rows* matrix, std::string const& name )
        { return column<int, NoCheck>(matrix, intMap_[name]); }

        template<bool NoCheck = false>
        static column<double, NoCheck> dblColumn( rows* matrix, std::string const& name )
        { return column<double, NoCheck>(matrix, dblMap_[name]); }

        template<bool NoCheck = false>
        static column<std::string, NoCheck> strColumn( rows* matrix, std::string const& name )
        { return column<std::string, NoCheck>(matrix, strMap_[name]); }
    
        template<typename T>
        using FieldMap = Utility::StringKeyMap<T Client::*, Utility::NoConv, Utility::DefaultThrow >;

    private:
        // Populating these requires template<> syntax, e.g.
        // template<>
        // Utility::ColumnAccess<Client>::FieldMap<int>
        // Utility::ColumnAccess<Client>::intMap_ = { ... };
        //
        static FieldMap<int>            intMap_;
        static FieldMap<double>         dblMap_;
        static FieldMap<std::string>    strMap_;
    };
    
    // AccessMap.
    // Mechanics of ColumnAccess for an individual type.
    // Instead of inheritance, the Client will need to define 
    // a wrapper method to disambiguate the column() call. 
    // See col2_example.cpp
    //
    template<typename Client, typename T>
    class AccessMap
    {
    public:
        using rows   = std::vector<Client>;

        template<bool NoCheck = false>
        using columnT = Column<T, Client, NoCheck>;
        
        template<bool NoCheck = false>
        static columnT<NoCheck> column( rows* matrix, std::string const& name )
        { return columnT<NoCheck>(matrix, map_[name]); }
    
    private:
        using FieldMap = Utility::StringKeyMap<T Client::*, Utility::NoConv, Utility::DefaultThrow >;
        // Populating these requires template<> syntax, e.g.
        // template<>
        // Utility::AccessMap<Client, int>::FieldMap
        // Utility::AccessMap<Client, int>::map_ = { ... };
        //
        static FieldMap     map_;
    };
    
} // namespace Utility

