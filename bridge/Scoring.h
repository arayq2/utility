#pragma once

#ifndef SWANGAMES_SCORING_H
#define SWANGAMES_SCORING_H

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

        Contract(Level l, Rank r, Dbld d, bool v)
        : level_(l)
        , rank_(r)
        , dbld_(d)
        , vul_(v)
        {}
        
        int score( int tricks ) const;

        Contract& level( Level l ) { level_ = l; return *this; }
        Contract& scale( Rank r )  { rank_  = r; return *this; }
        Contract& level( Dbld d )  { dbld_  = d; return *this; }
        Contract& level( bool v )  { vul_   = v; return *this; }

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
        int slams_() const;
    };


#endif // SWANGAMES_SCORING_H
