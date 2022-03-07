/** ======================================================================+
 + Copyright @2021-2022 Arjun Ray
 + Released under MIT License. See https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_CHECKLIST_H
#define UTILITY_CHECKLIST_H

#include <vector>
#include <algorithm>

namespace Utility
{
    template<typename Client>
    struct CheckListMethods
    {
        static void activate( Client& client, std::size_t counter )
        {
            client( counter ); // assumes function object
        }
    };

    /**
     * @class CheckList
     * @brief Activates function when all prerequisites are "marked".
     * Prerequisites are proxied by indexes in a bit set.
     * All marks are automatically cleared after activation.
     */
    template<typename Action, typename Methods = CheckListMethods<Action> >
    class CheckList
    {
        using BitSet = std::vector<bool>;
    public:
        ~CheckList() noexcept = default;

        CheckList(std::size_t size, Action const& action)
        : size_(size)
        , bset_(size_, false)
        , action_(action)
        {}

        // returns whether action was triggered
        bool mark( std::size_t index )
        {
            if ( index < size_ ) { bset_[index] = true; }
            return check_();
        }

        bool unmark( std::size_t index )
        {   // should be rare occurence
            if ( index < size_ )
            {
                bool    _was(bset_[index]);
                bset_[index] = false;
                return _was;
            }
            return false;
        }

        CheckList& set_counter( std::size_t start )
        {
            counter_ = start;
            return *this;
        }

        int marked() const
        {
            return std::count( bset_.begin(), bset_.end(), true );
        }

        int unmarked() const
        {
            return std::count( bset_.begin(), bset_.end(), false );
        }

    private:
        std::size_t size_;
        BitSet      bset_;
        Action      action_;
        std::size_t counter_{0};

        bool all_set_()
        {
            return bset_.end() == std::find( bset_.begin(), bset_.end(), false );
        }

        bool check_()
        {
            if ( all_set_() )
            {
                Methods::activate( action_, counter_ );
                reset_();
                return true;
            }
            return false;
        }
        
        void reset_()
        {
            bset_ = BitSet(size_, false);
            counter_++;
        }

        CheckList(CheckList const&) = delete;
        CheckList& operator=( CheckList const& ) = delete;
    };

} // namespace Utility

#endif // UTILITY_CHECKLIST_H

