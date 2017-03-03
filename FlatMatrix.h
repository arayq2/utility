#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace Utility
{
    // Policy classes for bounds checking
    template<bool> struct ColMethods;
    template<bool> struct RowMethods;
    
    // Specializations
    //
    template<> struct ColMethods<false>
    {
        template<typename VT>
        static VT& at_( size_t index, VT* vt, size_t size )
        {
           if ( index < size ) { return vt[index]; }
           throw std::out_of_range("ColMethods");
        }
        
        template<typename VT>
        static VT const& at_( size_t index, VT const* vt, size_t size )
        {
           if ( index < size ) { return vt[index]; }
           throw std::out_of_range("ColMethods");
        }
    };
    
    template<> struct ColMethods<true>
    {
        template<typename VT>
        static VT& at_( size_t index, VT* vt, size_t )
        {
           return vt[index];
        }
 
        template<typename VT>
        static VT const& at_( size_t index, VT const* vt, size_t )
        {
           return vt[index];
        }
    };
    
    template<> struct RowMethods<false>
    {
        template<typename Vector>
        static typename Vector::value_type& at_( size_t index, Vector& v )
        {
           return v.at( index );
        }
        
        template<typename Vector>
        static typename Vector::value_type const& at_( size_t index, Vector const& v )
        {
           return v.at( index );
        }
    };
    
    template<> struct RowMethods<true>
    {
        template<typename Vector>
        static typename Vector::value_type& at_( size_t index, Vector& v )
        {
           return v[index];
        }
 
        template<typename Vector>
        static typename Vector::value_type const& at_( size_t index, Vector const& v )
        {
           return v[index];
        }
    };
    
    template<typename T>
    std::ostream& dump_( std::ostream& os, T val )
    {
        return os.write( reinterpret_cast<char const*>(&val), sizeof(T) );
    }
    
    template<typename T>
    std::istream& load_( std::istream& is, T& val )
    {
        return is.read( reinterpret_cast<char*>(&val), sizeof(T) );
    }

    // FlatMatrix
    // Matrix facade on a memory block, with basic requirements only:
    // fixed dimensions known in advance,
    // access/mutate/iterate
    // stream insert/extract (binary)
    //
    template<
        typename DataType,   // usually numeric, possibly compound 
        bool SPEEDY = false  // default == check bounds
        >
    class FlatMatrix
    {
    public:
        class AsText
        {
        public:
            AsText(FlatMatrix const& matrix, char const* delim = ",", char const* eol = "\n")
            : matrix_(matrix)
            , delim_(delim)
            , eol_(eol)
            {}
            
            std::ostream& output( std::ostream& os ) const
            {
                for ( auto const& _row : matrix_ )
                {
                    auto    _ptr(_row.begin());
                    if ( _ptr != _row.end() )
                    {
                        os << *_ptr;
                        while ( ++_ptr != _row.end() ) { os << delim_ << *_ptr; }
                    }
                    os << eol_;
                }
                return os;
            }
            
            friend
            std::ostream& operator<<( std::ostream&  os, AsText const& astext )
            {
                return astext.output( os );
            }
            
        private:
            FlatMatrix const&   matrix_;
            char const*         delim_;
            char const*         eol_;
        };
        
        // ========================================================
        
        class Row
        {
        public:
            using value_type     = DataType;
            using iterator       = value_type*;
            using const_iterator = value_type const*;
            
            Row(size_t size, value_type* data = nullptr)
            : size_(size)
            , data_(data)
            {}
            
            value_type& operator[]( size_t index )
            {
                return ColMethods<SPEEDY>::at_( index, data_, size_ );
            }
            value_type const& operator[]( size_t index ) const
            {
                return ColMethods<SPEEDY>::at_( index, data_, size_ );
            }
        
            iterator begin() { return data_; }
            iterator end()   { return data_ + size_; }
        
            const_iterator begin() const { return data_; }
            const_iterator end()   const { return data_ + size_; }
            
            void reset( DataType const& value )
            {
                for ( auto& _slot : *this ) { _slot = value; }
            }
            
            size_t size() const { return size_; }
            
            friend class FlatMatrix;
        private:
            size_t          size_;
            value_type*     data_;
        };
        
        // ========================================================
        
        using value_type     = Row;
        using Vector         = std::vector<value_type>;
        using iterator       = typename Vector::iterator;
        using const_iterator = typename Vector::const_iterator;
        
        FlatMatrix(size_t rows, size_t cols, DataType* data = nullptr)
        : own_(data == nullptr)
        , size_(rows * cols)
        , data_(own_ ? new DataType[size_] : data)
        , vect_(rows, value_type(cols))
        {
            init_( cols, true );
        }
        
        FlatMatrix(std::istream& is)
        : own_(true)
        , size_(0)
        , data_(nullptr)
        , vect_()
        {
            size_t  _rows;
            size_t  _cols;
            if ( load_( is, _rows ) and load_( is, _cols ) )
            {
                size_ = _rows * _cols;
                data_ = new DataType[size_];
                if ( load( is ) )
                {
                    vect_.assign(_rows, value_type(_cols));
                    init_( _cols, false );
                }
            }
            // else throw error
        }
        
        ~FlatMatrix() noexcept
        {
            if ( own_ ) { delete [] data_; }
        }
        
        value_type& operator[]( size_t index )
        {
            return RowMethods<SPEEDY>::at_( index, vect_ );
        }
        value_type const& operator[]( size_t index ) const
        {
            return RowMethods<SPEEDY>::at_( index, vect_ );
        }
    
        iterator begin() { return vect_.begin(); }
        iterator end()   { return vect_.end(); }
    
        const_iterator begin() const { return vect_.begin(); }
        const_iterator end()   const { return vect_.end(); }
        
        void reset( DataType const& value = DataType() )
        {
            for ( auto& _row : vect_ ) { _row.reset( value ); }
        }
        
        // allow raw access to externally owned data only
        DataType* origin() { return own_ ? nullptr : data_; }
        
        size_t rows() const { return vect_.size(); }
        size_t cols() const { return (size_ / rows()); }
        size_t size() const { return size_; }
        
        std::ostream& dump( std::ostream& os, bool dims = false ) const
        {
             if ( dims )
             {
                dump_( os, rows() );
                dump_( os, cols() );
             }
             return os.write( reinterpret_cast<char const*>(data_), sizeof(DataType) * size_ );
        }
        
        friend
        std::ostream& operator<<( std::ostream& os, FlatMatrix const& m )
        {
            return m.dump( os, true );
        }
        
        // matrix needs to have been sized beforehand
        std::istream& load( std::istream& is )
        {
             return is.read( reinterpret_cast<char*>(data_), sizeof(DataType) * size_ );
        }
        
        friend 
        std::istream& operator>>( std::istream& os, FlatMatrix& m )
        {
            return m.load( os );
        }
                
    private:
        bool        own_;
        size_t      size_;
        DataType*   data_;
        Vector      vect_;
        
        FlatMatrix(FlatMatrix const&) = delete;
        FlatMatrix& operator=(FlatMatrix const&) = delete;
        
        void init_( size_t cols, bool reset )
        {
            DataType*   _ptr(data_);
            for ( auto& _row : vect_ )
            {
                _row.data_ = _ptr;
                if ( reset ) { _row.reset( DataType() ); }
                _ptr += cols;
            }
        }
    };

} // namespace Utility
