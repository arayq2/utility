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
        : rng_(std::forward<Args>(args)...)
        , max_(max)
        , slots_(max_)
        {
            reset();
        }

        int operator()()
        {
            if ( index_ == max_ ) { reset_(); }
            return slots_[index_++];
        }

        void reset()
        {
            std::iota( slots_.begin(), slots_.end(), 0 );
            reset_();
        }

    private:
        RNG     rng_;
        int     max_;
        int     index_{0};
        Vector  slots_;

        void reset_()
        {   // Fisher-Yates: permute slots
            for ( int _i{0}; _i < max_ - 1; ++_i )
            {
                std::swap( slots_[_i], slots_[rng_( _i, max_ - 1 )] );
            }
            index_ = 0;
        }
    };

} // namespace Utility
