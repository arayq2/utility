/** ======================================================================+
 + Copyright @2019-2023 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#include <random>
#include <vector>
#include <algorithm>

namespace Utility
{
    class MT19937
    {
    public:
        ~MT19937() noexcept = default;
        MT19937() = default;

        int operator()( int lo, int hi )
        {
            return std::uniform_int_distribution<int>{lo, hi}( mt_ ); 
        }

    private:
        std::random_device  rd_;
        std::mt19937_64     mt_{rd_()};
    };

    /**
     * @class Shuffler
     * @brief Fisher-Yates algorithm to shuffle an array of indices.
     */
    template<typename RNG = MT19937>
    class Shuffler
    {
    public:
        ~Shuffler() noexcept = default;
        Shuffler() = default;
        template<typename... Args>
        explicit
        Shuffler(int base, Args&&... args)
        : rng_(std::forward<Args>(args)...)
        , base_(base)
        {}

        void operator()( int* slots, int max, bool reinit = false )
        {
            if ( reinit )
            {
                std::iota( slots, slots + max, base_ );
            }
            for ( int _i{0}; _i < max - 1; ++_i )
            {
                std::swap( slots[_i], slots[rng_( _i, max - 1 )] );
            }
        }

    private:
        RNG     rng_;
        int     base_{0};
    };

    /**
     * @class Permuter
     * @brief returns indices of a random permutation, 0 to N-1.
     * Continues with a new permutation if needed.
     */
    template<typename RNG = MT19937>
    class Permuter
    {
        using Vector = std::vector<int>;
    public:
        ~Permuter() noexcept = default;
        template<typename... Args>
        explicit
        Permuter(int max, Args&&... args)
        : shuffler_(0, std::forward<Args>(args)...)
        , max_(max)
        , slots_(max_)
        {
            reset();
        }

        int operator()()
        {
            if ( index_ == max_ ) { reset_( false ); }
            return slots_[index_++];
        }

        void reset()
        {
            reset_( true );
        }

    private:
        Shuffler<RNG>   shuffler_;
        int             max_;
        int             index_{0};
        Vector          slots_;

        void reset_( bool reinit )
        {
            shuffler_( &slots_[0], max_, reinit );
            index_ = 0;
        }
    };

} // namespace Utility
