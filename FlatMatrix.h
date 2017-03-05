#pragma once

#include "DeclareException.h"
#include "SequenceJoin.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <cstring>

namespace Utility
{
    // Policy classes for bounds checking
    //
    template<bool> struct ColMethods;
    template<bool> struct RowMethods;
    
    // Specializations
    // false == check bounds, true == live dangerously
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
    
    // Utilities
    //
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
        DECLARE_EXCEPTION;
        DECLARE_SUBEXCEPTION(Dimensions);
        DECLARE_SUBEXCEPTION(Data);
        struct Index; // forward declaration of friend.
         
        // Row.
        // Minimal vector-like facade over memory block segment.
        //
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
                for ( auto& _cell : *this ) { _cell = value; }
            }
            
            size_t size() const { return size_; }
            
            friend class Index;
        private:
            size_t          size_;
            value_type*     data_;
        };
        
        using value_type     = Row;
        using Vector         = std::vector<value_type>;
        using iterator       = typename Vector::iterator;
        using const_iterator = typename Vector::const_iterator;
        
        FlatMatrix(size_t rows, size_t cols, DataType* data = nullptr)
        : sizer_(rows, cols)
        , block_(sizer_.size(), data)
        , index_(rows, cols, block_.origin())
        {}
        
        // assumes binary format: rows cols blob
        FlatMatrix(std::istream& is)
        : sizer_(is)
        , block_(sizer_.size())
        , index_(sizer_.rows_, sizer_.cols_, block_.origin())
        {
            load( is );
        }

        FlatMatrix(FlatMatrix const& model)
        : sizer_(model.sizer_)
        , block_(model.block_)
        , index_(sizer_.rows_, sizer_.cols_, block_.origin())
        {}
        
        FlatMatrix(FlatMatrix &&) = default;
        
        ~FlatMatrix() = default;
        
        FlatMatrix& operator=( FlatMatrix const& rhs )
        {
            FlatMatrix  _other(rhs);
            swap_( _other );
            return *this;
        }
        
        FlatMatrix& operator=( FlatMatrix&& rhs )
        {
            swap_( std::move(rhs) );
            return *this;
        }
        
        value_type& operator[]( size_t index )
        {
            return RowMethods<SPEEDY>::at_( index, index_.v_ );
        }
        value_type const& operator[]( size_t index ) const
        {
            return RowMethods<SPEEDY>::at_( index, index_.v_ );
        }
    
        void reset( DataType const& value = DataType() )
        {
            index_.reset( value );
        }
        
        // allow raw access to externally owned data only
        DataType* origin() { return block_.origin( true ); }
                
        iterator begin() { return index_.v_.begin(); }
        iterator end()   { return index_.v_.end(); }
    
        const_iterator begin() const { return index_.v_.begin(); }
        const_iterator end()   const { return index_.v_.end(); }
        
        size_t rows() const { return sizer_.rows_; }
        size_t cols() const { return sizer_.cols_; }
        size_t size() const { return sizer_.size(); }
        
        std::ostream& dump( std::ostream& os, bool dims = false ) const
        {
             if ( dims ) { sizer_.dump( os ); }
             return block_.dump( os );
        }
        
        friend
        std::ostream& operator<<( std::ostream& os, FlatMatrix const& m )
        {
            return m.dump( os, true );
        }
        
        // matrix needs to have been sized beforehand
        std::istream& load( std::istream& is )
        {
             if ( !block_.load( is ) ) { throw DataException(); }
             return is;
        }
        
        friend 
        std::istream& operator>>( std::istream& os, FlatMatrix& m )
        {
            return m.load( os );
        }
                
        // ========================================================

        // AsText.
        // Wrapper for formatted output. stream << AsText(flat_matrix_reference);
        //
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
                using Joiner = Utility::SequenceJoin<typename Row::const_iterator>;
                for ( auto const& _row : matrix_ )
                {
                    os << Joiner(_row.begin(), _row.end(), delim_) << eol_;
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
    private:
        // Sizer.
        // Manages dimensions information.
        //
        struct Sizer
        {
            size_t  rows_;
            size_t  cols_;
            
            Sizer(size_t rows, size_t cols)
            : rows_(rows)
            , cols_(cols)
            {}
            
            Sizer(std::istream& is)
            {
                if ( !(load_( is, rows_ ) and load_( is, cols_ )) )
                {
                    throw DimensionsException();
                }
            }
            
            std::ostream& dump( std::ostream& os ) const
            {
                dump_( os, rows_ );
                return dump_( os, cols_ );
            }
            
            size_t size() const { return (rows_ * cols_); }
        }           sizer_;
        // DataBlock.
        // Manages linear memory block.
        //
        class Block
        {
        public:
            Block(size_t size, DataType* data = nullptr)
            : own_(data == nullptr)
            , size_(size)
            , data_(own_ ? new DataType[size_]() : data)
            {}
            
            Block(Block const& model)
            : own_(true)
            , size_(model.size_)
            , data_(new DataType[size_])
            {
                try { std::copy( model.data_, model.data_ + size_, data_ ); }
                catch (...) { release_(); throw; }
            }

            ~Block() noexcept
            {
                if ( own_ ) { release_(); }
            }
            
            std::istream& load( std::istream& is )
            {
                return is.read( reinterpret_cast<char*>(data_), size_ * sizeof(DataType) );
            }
        
            std::ostream& dump( std::ostream& os ) const
            {
                return os.write( reinterpret_cast<char const*>(data_), size_ * sizeof(DataType) );
            }
            
            DataType* origin( bool check = false ) const
            {
                return (!check ? data_ : own_ ? nullptr : data_);
            }
        
        private:
            bool        own_;
            size_t      size_;
            DataType*   data_;
            
            Block& operator=( Block& ) = delete;
            
            void release_() { delete [] data_; }
            
        }           block_;
        // Index.
        // Manages memory block segmentation into "rows".
        //
        struct Index
        {
            Vector      v_;
            
            Index(size_t rows, size_t cols, DataType* data)
            : v_(rows, value_type(cols))
            {
                DataType*   _ptr(data);
                for ( auto& _row : v_ )
                {
                    _row.data_ = _ptr;
                    _ptr += cols;
                }
            }
            
            void reset( DataType const& value )
            {
                for ( auto& _row : v_ ) { _row.reset( value ); }
            }
            
            void swap( Index& other )
            {
                v_.swap( other.v_ );
            }

        }           index_;

        void swap_( FlatMatrix&& other )
        {
            using std::swap;
            swap( sizer_, other.sizer_ );
            swap( block_, other.block_ );
            index_.swap( other.index_ );
        }
    };

} // namespace Utility
