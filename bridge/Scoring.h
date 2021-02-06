#pragma once

#ifndef BRIDGE_SCORING_H
#define BRIDGE_SCORING_H

namespace bridge
{
    /**
     * @class Contract
     * @brief Captures specification of a bid in Contract Bridge.
     * Enum classes are used to enforce valid values only.
     * Level is the only class that needs numeric equivalents.
     */
    class Contract
    {
    public:
        enum class Level : int
        {
            L1, L2, L3, L4, L5, L6, L7
        };

        enum class Rank : int
        {
            NT, MN, MJ
        };

        enum class Dbld : int
        {
            NO, YES, AGAIN
        };

        Contract(Level l, Rank r, Dbld d, bool v = false)
        : level_(l)
        , rank_(r)
        , dbld_(d)
        , vul_(v)
        {}

        Contract(Level l, Rank r, bool v = false)
        : Contract(l, r, Dbld::NO, v)
        {}

        // Any number outside [0,13] returns 0
        int score( int tricks ) const;

        Contract& level( Level l ) { level_ = l; return *this; }
        Contract& rank( Rank r )   { rank_  = r; return *this; }
        Contract& dbld( Dbld d )   { dbld_  = d; return *this; }
        Contract& vul( bool v )    { vul_   = v; return *this; }

        Level level() const { return level_; }
        Rank  rank() const  { return rank_; }
        Dbld  dbld() const  { return dbld_; }
        bool  vul() const   { return vul_; }

    private:
        Level   level_;
        Rank    rank_;
        Dbld    dbld_;
        bool    vul_;
        //
        int minus_( int ut ) const; // undertricks
        int plus_( int ot = 0 ) const;  // overtricks
        int trickscore_( int ) const;
        int overtricks_( int ) const;
    };

} // namespace bridge

#endif // BRIDGE_SCORING_H
