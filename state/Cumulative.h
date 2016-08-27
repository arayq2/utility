#pragma once

#include <vector>
#include <algorithm>
#include <utility>

namespace Statistics
{
    // See notes on the implementation at the end of this file.
    
    using DVector = std::vector<double>;
    
    // policies to handle end() condition
    struct First  { static size_t index( size_t, size_t )        { return 0; } };
    struct Custom { static size_t index( size_t, size_t custom ) { return custom; } };
    struct Max    { static size_t index( size_t max, size_t )    { return max; } };
    struct Last   { static size_t index( size_t max, size_t )    { return max - 1; } };
        
    // Intercept.
    // Inverse cumulative function of a discrete distribution.
    // Finds 0-based index of CDF bucket bracketing a given value.
    //
    template<typename EndPolicy = Last>
    class Intercept
    {
    public:
        Intercept(DVector const& distribution, double divisor = 1.0);
        
        size_t operator() ( double ordinate, size_t custom = 0 ) const;
        size_t operator[] ( double ordinate ) const { return (*this)( ordinate, 0 ); }

    private:
        DVector const&  dist_;
        double const    scale_; // (\_sum(dist_))
    };
    
    // Cumulative.
    // Discrete distribution in un-normalized cumulative form.
    // Ordinate intercept will be scaled appropriately.
    //
    class Cumulative
    {
    public:
        using Bucket  = std::pair<double, size_t>;
        using BVector = std::vector<Bucket>;
        
        Cumulative();
        
        template<typename Container>
        explicit
        Cumulative(Container const& source);
        
        template<typename Iterator>
        Cumulative(Iterator first, Iterator last);
        
        // permits stepwise construction
        void reserve( size_t size );
        void bump();
        void append( double interval );
        
        // comparison function for binary search.
        // override default lexicographic behavior with std::pair.
        static 
        bool lessthan( Bucket const& lhs, Bucket const& rhs )
        {
            return lhs.first < rhs.first;
        }
        
        // index of interval bucketing this ordinate value
        size_t invert( double ordinate ) const;
        
        template<typename EndPolicy = Custom>
        size_t invert( double ordinate, size_t custom = 0 ) const;
        
        // use uniform [0,1) distribution rv generator directly
        template<typename Urvg>
        size_t draw( Urvg& sampler ) const;
        
        // fill vector with implied density distribution
        size_t densities( DVector& df ) const;
        DVector densities() const;
        
        class Inserter
        : public std::iterator<std::output_iterator_tag, void, void, void, void>
        {
        public:
            using value_type = double;
            
            explicit
            Inserter(Cumulative* cdf) : cdf_(cdf) {}
            
            Inserter& operator* () { return *this; }
            Inserter& operator++ () { return *this; }
            Inserter& operator= ( value_type );
            
        private:
            Cumulative*     cdf_;
        };
        
        Inserter inserter() { return Inserter(this); }
        
        Inserter inserter( size_t size );

        private:
        template<typename Iterator>
        void fill( Iterator first, Iterator last );
    
        BVector     buckets_;
        double      scale_;
        size_t      max_;
    };
    
    // ==[ Implementation ]================================================
    
    template<typename EndPolicy>
    inline
    Intercept<EndPolicy>::Intercept(DVector const& distribution, double divisor)
    : dist_(distribution)
    , scale_(divisor)
    {}
    
    template<typename EndPolicy>
    inline
    size_t Intercept<EndPolicy>::operator() ( double ordinate, size_t custom ) const
    {
        size_t  _index(0);
        double  _total(0);
        double  _mark(ordinate * scale_);
        
        for ( auto const& _value : dist_ )
        {
            if ( _value > 0 )
            {
                _total += _value;
                if ( _mark < _total ) { return _index; }
            }
            ++_index;
        }
        return EndPolicy::index( _index, custom );
    };

    ///////////////////////////////////////////////////////////////////////

    inline
    Cumulative::Cumulative()
    : Cumulative::Cumulative(DVector())
    {}

    template<typename Container>
    inline
    Cumulative::Cumulative(Container const& container)
    : Cumulative::Cumulative(container.begin(), container.end())
    {}
        
    template<typename Iterator>
    inline
    Cumulative::Cumulative(Iterator first, Iterator last)
    : buckets_()
    , scale_(0.0)
    , max_(0)
    {
        buckets_.reserve( std::distance( first, last ) );
        fill( first, last );
    }
    
    template<typename Iterator>
    inline void 
    Cumulative::fill( Iterator first, Iterator last )
    {
        std::copy( first, last, inserter() );
    }
    
    inline void 
    Cumulative::reserve( size_t size ) { buckets_.reserve( size ); }
    
    inline void 
    Cumulative::bump() { ++max_; }
    
    inline void 
    Cumulative::append( double interval )
    {
        if ( interval > 0 )
        {
            scale_ += interval;
            buckets_.push_back( std::make_pair( scale_, max_ ) );
        }
        // Note: this also accounts for zero-length intervals
        bump();
    }
    
    inline size_t
    Cumulative::invert( double ordinate ) const
    {
        auto    _bptr(std::upper_bound( buckets_.begin(), buckets_.end(),
                    std::make_pair( ordinate * scale_, 0 ), lessthan ));
        return _bptr != buckets_.end() ? _bptr->second : max_;
    }
    
    template<typename EndPolicy>
    inline size_t
    Cumulative::invert( double ordinate, size_t custom ) const
    {
        auto    _bptr(std::upper_bound( buckets_.begin(), buckets_.end(),
                    std::make_pair( ordinate * scale_, 0 ), lessthan ));
        return _bptr != buckets_.end()
        ? _bptr->second 
        : EndPolicy::index( max_, custom )
        ;
    }
    
    template<typename Urvg>
    inline size_t
    Cumulative::draw( Urvg& sampler ) const
    {
        return invert( sampler() );
    }

    inline size_t
    Cumulative::densities( DVector& df ) const
    {
        double  _prev(0.0);
        for ( auto const& _bucket : buckets_ )
        {
            df.push_back( (_bucket.first - _prev) / scale_ );
            _prev = _bucket.first;
        }
        return buckets_.size();
    }
    
    // compiler should apply RVO here.
    inline DVector
    Cumulative::densities() const
    {
        DVector     _pdf;
        densities( _pdf );
        return _pdf;
    }

    inline Cumulative::Inserter
    Cumulative::inserter( size_t size )
    {
        buckets_.reserve( size );
        return Inserter(this);
    }

    inline Cumulative::Inserter&
    Cumulative::Inserter::operator= ( double step )
    {
        cdf_->append( step );
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////
#if 0
    NOTES
    
    1. The basic idea of sampling from a probability distribution is to 
    treat a uniform [0,1) random variable as the ordinate of the cumulative 
    distribution function and determine the corresponding abscissa. This
    algorithm is implemented in Intercept<EndPolicy>::operator(), where 
    the cumulative distribution is (partially) computed on the fly.
    
    2. However, the empirical distributions often used in simulations are
    usually computed in un-normalized form first, e.g. as a set of odds 
    ratios from bivariate logistic regressions.  For sampling, normalizing 
    these distributions is unnecessary. Instead the sum can be maintained
    internally as a scale factor, while the partial sums computed on the 
    way to the total are exactly the values required for a search using 
    std::upper_bound(). The Transition::Cumulative class implements this,
    and saves a number of division operations at the cost of a single
    multiplication (scaling the ordinate for the search).
#endif    
} // namespace Statistics
