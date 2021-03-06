
#include "Scoring.h"

namespace bridge
{
namespace
{
    inline
    int as_integer( Contract::Level level )
    {
        switch ( level )
        {
        default:
        case Contract::Level::L1 : return 1;
        case Contract::Level::L2 : return 2;
        case Contract::Level::L3 : return 3;
        case Contract::Level::L4 : return 4;
        case Contract::Level::L5 : return 5;
        case Contract::Level::L6 : return 6;
        case Contract::Level::L7 : return 7;
        }
    }

    inline
    int slam( Contract::Level level, bool vul )
    {
        switch ( level )
        {
        case Contract::Level::L7 : return vul ? 1500 : 1000;
        case Contract::Level::L6 : return vul ?  750 :  500;
        default: return 0;
        }
    }
}

    int
    Contract::score( int tricks ) const
    {
        if ( tricks < 0 || tricks > 13 ) { return 0; }
        int _diff(tricks - 6 - as_integer( level_ ));
        return _diff < 0 ? -minus_( -_diff ) : plus_( _diff );
    }

    int
    Contract::minus_( int ut ) const
    {
        switch ( dbld_ )
        {
        case Dbld::NO   :
            return ut * (vul_ ? 100 : 50);
        case Dbld::YES  :
            return vul_ ? 200 + (ut - 1) * 300 : 100 + (ut - 1) * 200 + (ut > 3 ? ut - 3 : 0) * 100;
        case Dbld::AGAIN:
            return vul_ ? 400 + (ut - 1) * 600 : 200 + (ut - 1) * 400 + (ut > 3 ? ut - 3 : 0) * 200;
        default : return 0;
        }
    }

    int
    Contract::trickscore_( int tricks ) const
    {
        switch ( rank_ )
        {
        case Rank::NT : return 40 + 30 * (tricks - 1);
        case Rank::MN : return 20 * tricks;
        case Rank::MJ : return 30 * tricks;
        default : return 0;
        }
    }
    
    int
    Contract::overtricks_( int ot ) const
    {
        switch ( dbld_ )
        {
        case Dbld::NO   :
            switch ( rank_ )
            {
            case Rank::NT :
            case Rank::MJ : return 30 * ot;
            case Rank::MN : return 20 * ot;
            }
        case Dbld::YES  : return ot * (vul_ ? 200 : 100);
        case Dbld::AGAIN: return ot * (vul_ ? 400 : 200);
        default : return 0;
        }
    }
    
    int
    Contract::plus_( int ot ) const
    {
        // trickscore
        int _ts;
        int _xtra;
        switch ( dbld_ )
        {
        case Dbld::NO   : 
            _ts = trickscore_( as_integer( level_ ) );
            _xtra = 0;
            break;
        case Dbld::YES  :
            _ts = 2 * trickscore_( as_integer( level_ ) );
            _xtra = 50;
            break;
        case Dbld::AGAIN:
            _ts = 4 * trickscore_( as_integer( level_ ) );
            _xtra = 100;
            break;
        }
        // overtricks
        int _otb = overtricks_( ot );
        // bonuses
        int _game = (_ts < 100 ? 50 : vul_ ? 500 : 300);
        // total
        return _ts + _xtra + _otb + _game + slam( level_, vul_ );
    }

} // namespace bridge
