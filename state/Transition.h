#pragma once

#include "Cumulative.h"
#include "DeclareException.h"

namespace Transition
{
    // Transition.
    // Sparse matrix implementation of a transition function.
    // Suitable for dynamic assembly of a probability distribution 
    // based on odds ratio calculations (e.g. Begg-Gray).

    class Runtime;  // parameter wrapper, probably type-erased
    class OddsCalc; // external functor
    
    template<typename OddsCalc> 
    struct OddsCalcMethods
    {
        static double 
        invoke( OddsCalc*, Runtime&, size_t );
    };

    class Builder;  // creates run time instances

    using StateId = size_t;
    
    // Cell.
    // Encapsulates odds ratio calculations.
    //
    class Cell
    {
    public:
        // no public ctor (see below)
        Cell(Cell const&) = default;
        Cell& operator= (Cell const&) = default;
        
        // invokes odds ratio calculation
        double operator() ( Runtime& rt ) const;
        
        StateId id() const { return to_; }
    
    private:
        StateId     to_; // real index
        OddsCalc*   calc_;
        
        Cell(StateId to, OddsCalc* calc);
        
        friend class Builder;
    };
    
    // Row.
    // Aggregates TO states for FROM state. Invalid transitions excluded.
    //
    class Row
    {
    public:
        using CVector = std::vector<Cell>;
        using iterator = CVector::const_iterator;
        
        // no public ctor (see below)
        Row(Row const&) = default;
        Row& operator= (Row const&) = default;
        
        // translate notional index to state id
        StateId resolve( size_t index ) const;
        
        // iterate over TO states (e.g. to assemble distribution)
        template<typename Action>
        void apply( Action&& action ) const;
        
        size_t size() const;
    
        iterator begin() const { return toStates_.cbegin(); }
        iterator end()   const { return toStates_.cend(); }
        
    private:
        StateId     from_; // real index
        CVector     toStates_;
        
        Row(StateId from);
        
        friend class Builder;
    };
    
    // Function
    // Basic functionality of this module
    //
    class Function
    {
    public:
        using Cdf = Statistics::Cumulative;
        
        DECLARE_EXCEPTION;

        // assemble distribution
        Function(Row const* row, Runtime& rt);
        
        // sample and resolve
        StateId operator() ( double urv ) const;
        StateId operator() ( double urv, bool check /*discriminator*/ ) const;
        
        Function(Function const&) = default;
        void swap( Function& other );
        Function& operator= ( Function rhs ); // copy and swap idiom
        
    private:
        Row const*  row_;
        Cdf         cdf_;
        
        double validate( double val ) const; // enforce [0,1)
    };
    
    // Matrix
    // Aggregates FROM states. Absorbing states excluded.
    //
    class Matrix
    {
    public:
        using RVector = std::vector<Row>;
        
        // no public ctor (see below)
        Matrix(Matrix const&) = default;
        Matrix& operator= (Matrix const&) = default;
        
        Function sampler( StateId from, Runtime& rt ) const;

        StateId step( StateId from, Runtime& rt, double probability ) const;

        // same as above but enforces probability in [0,1)
        StateId step( StateId from, Runtime& rt, double probability, bool check ) const;

    private:
        RVector     fromStates_;
        
        Matrix() = default;

        friend class Builder;
    };

    // ==[ Implementation ]================================================
    
    inline
    Cell::Cell(StateId to, OddsCalc* calc)
    : to_(to)
    , calc_(calc)
    {}
    
    inline double
    Cell::operator() ( Runtime& rt ) const
    {
        return OddsCalcMethods<OddsCalc>::invoke( calc_, rt, to_ );
    }

    ///////////////////////////////////////////////////////////////////////
    
    inline
    Row::Row(StateId from)
    : from_(from)
    {}

    inline StateId
    Row::resolve( size_t index ) const
    {
        // residual probability (if any) => no transition
        return index < toStates_.size() ? toStates_[index].id() : from_;
    }

    template<typename Action>
    inline void
    Row::apply( Action&& action ) const
    {
        for ( auto const& _cell : toStates_ ) { action( _cell ); }
    }
    
    inline size_t
    Row::size() const
    {
        return toStates_.size();
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    inline
    Function::Function(Row const* row, Runtime& rt)
    : row_(row)
    , cdf_()
    {
        cdf_.reserve( row_->size() );
        for ( auto& cell : *row_ ) { cdf_.append( cell( rt ) ); }
    }
    
    inline void
    Function::swap( Function& other )
    {
        using std::swap;
        swap( row_, other.row_ );
        cdf_.swap( other.cdf_ );
    }
    
    inline Function&
    Function::operator= ( Function rhs )
    {
        swap( rhs );
        return *this;
    }
    
    inline StateId
    Function::operator() ( double urv ) const
    {
        return row_->resolve( cdf_.invert( urv ) );
    }

    inline StateId
    Function::operator() ( double urv, bool ) const
    {
        return row_->resolve( cdf_.invert( validate( urv ) ) );
    }

    inline double
    Function::validate( double val ) const
    {
        if ( (val < 0.0) or !(val < 1.0) ) 
        {
            throw Exception("Invalid sample probability");
        }
        return val;
    }

    ///////////////////////////////////////////////////////////////////////

    inline Function
    Matrix::sampler( StateId from, Runtime& rt ) const
    {
        return Function(&fromStates_.at( from ), rt);
    }

    inline StateId
    Matrix::step( StateId from, Runtime& rt, double probability ) const
    {
        return sampler( from, rt )( probability );
    }

    inline StateId
    Matrix::step( StateId from, Runtime& rt, double probability, bool check ) const
    {
        return sampler( from, rt )( probability, check );
    }

} // namespace Transition
