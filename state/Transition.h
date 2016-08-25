#pragma once

#include "Cumulative.h"
#include "DeclareException.h"

    class OddsCalc; // external functor

namespace Transition
{
    // Transition.
    // Sparse matrix implementation of a transition function.
    // Suitable for dynamic assembly of a probability distribution 
    // based on odds ratio calculations (e.g. Begg-Gray).

    class Builder; // creates run time instances

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
        template<typename Runtime>
        double operator() ( Runtime& rt ) const;
        
        StateId id() const { return to_; }
    
    private:
        StateId     to_; // real index
        OddsCalc*   calc_;
        
        friend class Builder;
        Cell(StateId to, OddsCalc* calc);
        
    };
    
    // Row.
    // Aggregates TO states for FROM state. Invalid transitions excluded.
    //
    class Row
    {
    public:
        using CVector = std::vector<Cell>;
        
        // no public ctor (see below)
        Row(Row const&) = default;
        Row& operator= (Row const&) = default;
        
        // inserts odds ratios into target (vector)
        template<typename Inserter, typename Runtime>
        void traverse( Inserter& inserter, Runtime& rt ) const;
        
        // translates notional index to state id
        StateId resolve( size_t index ) const;
        
        // assemble distribution and resolve intercept
        template<typename Runtime>
        StateId transit( Runtime& rt, double prob ) const;
    
        template<typename Action>
        void apply( Action&& action ) const;
    
    private:
        StateId     from_; // real index
        CVector     toStates_;
        
        friend class Builder;
        Row(StateId from);
        
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
        
        DECLARE_EXCEPTION;

        template<typename Runtime>
        StateId step( StateId from, Runtime& rt, double samplerVal ) const;

    private:
        RVector     fromStates_;
        struct URV
        {
            double operator() ( double val ) const; // enforce [0,1)
        }           validator_;
        
        friend class Builder;
        Matrix() = default; 
    };

    // ==[ Implementation ]================================================
    
    inline
    Cell::Cell(StateId to, OddsCalc* calc)
    : to_(to)
    , calc_(calc)
    {}
    
    template<typename Runtime>
    inline double
    Cell::operator() ( Runtime& rt ) const
    {
        return (*calc_)( rt, to_ );
    }

    ///////////////////////////////////////////////////////////////////////
    
    inline
    Row::Row(StateId from)
    : from_(from)
    {}
    
    template<typename Inserter, typename Runtime>
    inline void
    Row::traverse( Inserter& inserter, Runtime& rt ) const
    {
        for ( auto const& _cell : toStates_ )
        {
            *inserter = _cell( rt );
            ++inserter;
        }
    }

    inline StateId
    Row::resolve( size_t index ) const
    {
        // residual probability (if any) => no transition
        return index < toStates_.size() ? toStates_[index].id() : from_;
    }

    template<typename Runtime>
    inline StateId
    Row::transit( Runtime& rt, double prob ) const
    {
        ProbDist::Cumulative    _cdf;
        // assemble distribution
        traverse( _cdf.inserter( toStates_.size() ), rt );
        // sample and interpret result
        return resolve( _cdf.index( prob ) );
    }
    
    template<typename Action>
    inline void
    Row::apply( Action&& action ) const
    {
        for ( auto const& _cell : toStates_ ) { action( _cell ); }
    }
    
    ///////////////////////////////////////////////////////////////////////

    template<typename Runtime>
    inline StateId
    Matrix::step( StateId from, Runtime& rt, double samplerVal ) const
    {
        return fromStates_.at( from ).transit( rt, validator_( samplerVal ) );
    }

    inline double
    Matrix::URV::operator() ( double val ) const
    {
        if ( (val < 0.0) or !(val < 1.0) ) 
        {
            throw Exception("Invalid probability");
        }
        return val;
    }
    
} // namespace Transition
