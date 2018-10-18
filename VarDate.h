#ifndef VARDATE_H
#define VARDATE_H

    /**
     * VarDate. (Variant Date) 
     * Copyright 2004-2016 Arjun Ray.
     *
     * Excel and COM share the "VARIANT date" definition, which 
     * originally had the bug of treating 1900 as a leap year. 
     * Later, the original day 0 of 31-Dec-1899 was "adjusted" 
     * to 30-Dec-1899.  Here, the first 59 days of 1900 are off 
     * by one, because the extra fix is not worth the effort.
     *
     * There are no lookup tables. All constants are hard coded,  
     * as date arithmetic is inherently magical.
     *     
     * See Notes at the end of the file.
     */

    class VarDate {
        // magic constants
        enum 
        {
            // first two months of 1600 CE (nominally)
            VARDATE_ADJUST = 59,
            // datum 0 = 29-Feb-1600, last day of cycle 
            VARDATE_OFFSET = 1600,
            // datum for 30-Dec-1899 == VARIANT date 0
            VARDATE_ZERO   = 109512,
            // total number of days in 400-year cycle
            VARDATE_CYCLE  = 146097
        };
        
    public:
    // constructors
        VarDate() : y_(0), m_(0), d_(0), datum_(0) {}
        
        VarDate( int y, int m, int d ) : y_(y), m_(m), d_(d), datum_(datum()) {}

        VarDate( double comDate )
        {
            parseDatum_( (long)comDate + VARDATE_ZERO );
            y_ += VARDATE_OFFSET;
        }

        VarDate( int datum, bool datumDate = false )
        {
            parseDatum_( datum + (datumDate ? 0 : VARDATE_ZERO) );
            y_ += VARDATE_OFFSET;
        }

        VarDate( char const* y4m2d2 )
        {
            if ( !parseYMD( y4m2d2, &y_, &m_, &d_ ) )
            {
                datum_ = y_ = m_ = d_ = 0;
            }
            else
            {
                datum_ = datum();
            }
        }

        ~VarDate() {} 

    // assignment
        VarDate& operator= ( char *y4m2d2 )
        {
            if ( !parseYMD( y4m2d2, &y_, &m_, &d_ ) )
            {
                datum_ = y_ = m_ = d_ = 0;
            }
            else
            {
                datum_ = datum();
            }
            return *this;
        }

        VarDate& operator= ( double comDate )
        {
            parseDatum_( (int)comDate + VARDATE_ZERO );
            y_ += VARDATE_OFFSET;
            return *this;
        }

        VarDate& operator= ( int datum )
        {
            parseDatum_( datum );
            y_ += VARDATE_OFFSET;
            return *this;
        }

        VarDate& operator= ( VarDate& rhs )
        {
            y_ = rhs.y_ ; m_ = rhs.m_ ; d_ = rhs.d_; 
            return *this;
        }

    // arithmetic
        int operator- ( VarDate& rhs ) const
        {
            return datum_ - rhs.datum_;
        }

        VarDate& shift( int days )
        {
            parseDatum_( datum_ + days );
            y_ += VARDATE_OFFSET;
            return *this;
        }

    // logical
        bool operator== ( VarDate& rhs ) const
        {
            return datum_ == rhs.datum_;
        }

        bool operator! () const
        {
            return y_ == 0 || m_ == 0 || d_ == 0;
        }

    // output
        template<typename Y, typename M, typename D> // various kinds of int
        VarDate& get( Y& yr, M& mo, D& da )
        {
            yr = (Y)y_; mo = (M)m_; da = (D)d_;
            return *this;
        }


        double variantDate() const
        {
            return (double) (datum_ - VARDATE_ZERO);
        }

        int datum() const
        {
            return rawCount( y_ - VARDATE_OFFSET, m_, d_ );
        }
        
        int dow() const // Sunday == 0, datum 0 == Tuesday
        {
            return ((datum_ + 2) % 7);
        }

    // utilities
        static inline
        int isLeap( int year ) 
        { 
            return year % 4 ? 0 : year % 100 ? 1 : year % 400 ? 0 : 1; 
        }
        
        static inline
        int leapDays( int year ) 
        { 
            return ( year / 4 - year / 100 + year / 400 ); 
        }

        static inline
        int maxdays( int month, int year )
        {
            switch ( month )
            {
            case  1: case  3: case  5:
            case  7: case  8: case 10: case 12: return 31;
            case  4: case  6: case  9: case 11: return 30;
            case  2: return 28 + isLeap( year );            
            }
        }
        
        static inline
        bool isValid( int yr, int mo, int dy )
        {
            return !(yr < 1 || mo < 1 || mo > 12 || dy < 1 || dy > maxdays( mo, yr ));
        }

        static inline
        int daysInYear( int m, int d )
        {
            switch ( m )
            {
            case  1: return d;       case  2: return d +  31; case  3: return d +  59;
            case  4: return d +  90; case  5: return d + 120; case  6: return d + 151;
            case  7: return d + 181; case  8: return d + 212; case  9: return d + 243;
            case 10: return d + 273; case 11: return d + 304; case 12: return d + 334;
            }
        }
        
        static inline
        int yearDays( int yr )
        {
            return yr * 365 + leapDays( yr ) - VARDATE_ADJUST;
        }

        static inline
        int rawCount( int yr, int mo, int da ) 
        {
            switch ( mo )
            {
            case  1: case  2: return daysInYear( mo, da ) + yearDays( yr ) - isLeap( yr );
            case  3: case  4: case  5: case  6:
            case  7: case  8: case  9: case 10:
            case 11: case 12: return daysInYear( mo, da ) + yearDays( yr );
            default: return 0;
            }
        }
        
        static inline
        int toDigit( char const data )
        {
            return (data - '0');
        }
        
        static inline
        int qAtol( char const* data, int len )
        {
            int result = toDigit( *data );
            while ( --len )
            {
                ++data;
                result *= 10;
                result += toDigit( *data );
            }
            return result;
        }

        static inline
        bool parseYMD( char const* date, int* yr, int* mo, int *dy )
        {

            *yr = qAtol( date, 4 );
            *mo = qAtol( date + 4, 2 );
            *dy = qAtol( date + 6, 2 );
            return isValid( *yr, *mo, *dy );
        }

        static inline
        int monthOfPlus( int& days )
        {
            // binary search
            if ( days > 184 )
            {
                if ( days > 275 )
                { 
                    if ( days > 337 ) { days -= 337; return  2; } // Feb
                    else
                    if ( days > 306 ) { days -= 306; return  1; } // Jan
                    else              { days -= 275; return 12; } // Dec
                }
                else
                {
                    if ( days > 245 ) { days -= 245; return 11; } // Nov
                    else
                    if ( days > 214 ) { days -= 214; return 10; } // Oct
                    else              { days -= 184; return  9; } // Sep
                }
            }
            else
            {
                if ( days > 92 )
                {
                    if ( days > 153 ) { days -= 153; return  8; } // Aug
                    else
                    if ( days > 122 ) { days -= 122; return  7; } // Jul
                    else              { days -=  92; return  6; } // Jun
                }
                else
                {
                    if ( days >  61 ) { days -=  61; return  5; } // May
                    else
                    if ( days >  31 ) { days -=  31; return  4; } // Apr
                    else              {              return  3; } // Mar
                }
            }
        }
        
        static inline
        int monthOfMinus( int& days )
        {
            // binary search: max(leap days) == 97 in 400 year cycle
            if ( days > -59 )
            {
                if ( days > -28 ) { days +=  28; return  2; } // Feb
                else              { days +=  59; return  1; } // Jan
            }
            else
            {
                if ( days > -90 ) { days +=  90; return 12; } // Dec
                else              { days += 120; return 11; } // Nov
            }
        }
        
    private:
        int   y_; // year
        int   m_; // month
        int   d_; // date
        int   datum_; // days since 29-Feb-1600

        void parseDatum_( long datum )
        {
            int epoch = datum / VARDATE_CYCLE;
            datum_ = datum % VARDATE_CYCLE;
            y_ = datum_ / 365;
            d_ = datum_ % 365 - leapDays( y_ ); // 0 == last day of Feb
            
            if ( d_ > 0 )
            {
                m_ = monthOfPlus( d_ );
                if ( m_ < 3 ) { ++y_; }
            }
            else
            {
                m_ = monthOfMinus( d_ += isLeap( y_ ) );
                if ( m_ > 2 ) { --y_; }
            }
            y_ += 400 * epoch;
        }
        
        friend class FormatDate;

    };

#include <stdio.h>

    class FormatDate
    {
    public: 
        enum Style { BLANK = 0, Y4MD, DMY_US, DMY_US0, DMY_EU, DMY_EU0, LITMO, ISO };

        FormatDate(VarDate const& vdate, Style style = Y4MD) { format_( vdate, style ); }
        
        operator char const* () const { return buf_; }
        char const* get() const  { return buf_; }

    private:
        enum { B_SIZE = 16 };
        char    buf_[B_SIZE];
        
        size_t format_( VarDate const& vd, Style style )
        {
            switch ( style )
            {
            default:
            case Y4MD:    return sprintf( buf_, "%4d%02d%02d", vd.y_, vd.m_, vd.d_ );
            case DMY_US:  return sprintf( buf_, "%2d/%2d/%4d", vd.m_, vd.d_, vd.y_ );
            case DMY_US0: return sprintf( buf_, "%02d/%02d/%4d", vd.m_, vd.d_, vd.y_ );
            case DMY_EU:  return sprintf( buf_, "%2d/%2d/%4d", vd.d_, vd.m_, vd.y_ );
            case DMY_EU0: return sprintf( buf_, "%02d/%02d/%4d", vd.d_, vd.m_, vd.y_ );
            case LITMO:   return sprintf( buf_, "%02d-%3s-%4d", vd.d_, month_( vd.m_ ), vd.y_ );
            case ISO:     return sprintf( buf_, "%4d-%02d-%02d", vd.y_, vd.m_, vd.d_ );
            }
        }
        
        char const* month_( int month ) const
        {
            switch ( month )
            {
            case  1: return "Jan"; case  2: return "Feb"; case  3: return "Mar";
            case  4: return "Apr"; case  5: return "May"; case  6: return "Jun";
            case  7: return "Jul"; case  8: return "Aug"; case  9: return "Sep";
            case 10: return "Oct"; case 11: return "Nov"; case 12: return "Dec";
            default: return "???"; // should never happen
            }
        }
    };
    
    
#endif

/* ==[ NOTES ]================================================ +

    This is not completely general, for performance reasons.  
    It would be if parseDatum were invoked only on datum % 146097 
    (== days in 400 years), adjusted to be positive, e.g.

        int epoch = rawDatum / 146097 ;
        int datum = rawDatum % 146097 ;
        if ( datum < 0 ) { datum += 146097 ; epoch-- ; }
        parseDatum( datum ) ;
        y_ += 400 * epoch ;

        and finally, as usual,
        y_ += DATUM_BASE_YEAR ;

    where DATUM_BASE_YEAR % 400 == 0

    Choosing a base year 0 mod 400 makes the math in isLeap() and 
    leapDays() completely regular and efficient.  

    We choose DATUM_BASE_YEAR as 1600, rather than 2000, because  
    of practicality: we need to handle dates in the (late) 1900's.

    The algorithm in parseDatum_() is based on treating 
    
        1-Mar-DATUM_BASE_YEAR == 1
        
    in turn based on treating logical years as ending on the last
    day of February of the same calendar year. Thus the initial
    value of d_ will always be logical 0 for this particular day, 
    in terms of logical years.  We catch the leap year case neatly 
    in the else-branch of the final adjustment, as the only way to 
    get Feb 29 is to have d_ == 0 and isLeap(y_) == 1. 


    Factoid: 146097 % 7 == 0.  That is, the leap year cycle over
    400 years also spans exactly 20871 weeks.  Hmm...

 + =========================================================== */
