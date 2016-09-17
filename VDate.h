#ifndef VDATE_H
#define VDATE_H

    /**
	 * VDate. (Variant Date) 
	 * Copyright 2004-2016 Arjun Ray.
	 * Excel uses the COM "variant date" definition, which had
	 * the bug that 1900 was treated as a leap year.  Thus the
	 * original day 0 of 31-Dec-1899 had to be "adjusted" to
	 * 30-Dec-1899. The first 59 days of 1900 are off by one.
	 * See Notes at the end of the file.
	 */

#include <stdio.h>
// datum: 0 == 29-Feb-1600, last day of 400 year cycle,
#define VDATE_OFFSET	1600
// VDATE_ZERO is datum for 30-Dec-1899 == Variant Date 0
#define VDATE_ZERO		109512
// 59 for treating year 1600 == 0 as normal in rawCount
#define VDATE_MAGIC	59

#define TO_DIGIT(x)		((x) - '0')

#define VDATE_DAY_V() \
static const long ds[] = { 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 28 } ;


#define VDATE_COUNT_V() \
static const long days[] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 } ;

#define VDATE_MONTH_V() \
static const char* months[] = { "", "Jan", "Feb", "Mar", "Apr", "May", \
			"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" } ;

struct VDate {
	long	y_ ;
	long	m_ ;
	long	d_ ;
	enum	style { BLANK = 0, Y4MD, DMY_US, DMY_US0, DMY_EU, DMY_EU0, LITMO, ISO }
			f_ ;
	size_t	l_ ;
	char	b_[16] ;

// constructors
	VDate() : y_(0), m_(0), d_(0), f_(BLANK) {}
	
	VDate( long y, long m, long d ) : y_(y), m_(m), d_(d), f_(BLANK) {}

	VDate( double comDate ) : f_(BLANK)
	{
		parseDatum( (long)comDate + VDATE_ZERO ) ;
		y_ += VDATE_OFFSET ;
	}

	VDate( long datum ) : f_(BLANK)
	{
		parseDatum( datum ) ;
		y_ += VDATE_OFFSET ;
	}

	VDate( char* y4m2d2 ) : f_(BLANK) 
	{
		if ( !parseYMD( y4m2d2, &y_, &m_, &d_ ) ) {
			y_ = m_ = d_ = 0 ;
		}
	}

	virtual ~VDate() {} 

// assignment
	VDate& operator= ( char *y4m2d2 )
	{
		if ( !parseYMD( y4m2d2, &y_, &m_, &d_ ) ) {
			y_ = m_ = d_ = 0 ;
		}
		f_ = BLANK ;
		return *this ;
	}

	VDate& operator= ( double comDate )
	{
		parseDatum( (long)comDate + VDATE_ZERO ) ;
		y_ += VDATE_OFFSET ;
		f_ = BLANK ;
		return *this ;
	}

	VDate& operator= ( long datum )
	{
		parseDatum( datum ) ;
		y_ += VDATE_OFFSET ;
		f_ = BLANK ;
		return *this ;
	}

	VDate& operator= ( VDate& rhs )
	{
		y_ = rhs.y_ ; m_ = rhs.m_ ; d_ = rhs.d_ ; 
		if ( (f_ = rhs.f_) != BLANK ) {
			l_ = rhs.l_ ;
			char* to = b_ ; char* from = rhs.b_ ;
			while ( *to++ = *from++ ) ;
		}
		return *this ;
	}

// arithmetic
	long operator- ( VDate& rhs )
	{
		return datum() - rhs.datum() ;
	}

	VDate& shift( long days )
	{
		parseDatum( datum() + days ) ;
		y_ += VDATE_OFFSET ;
		f_ = BLANK ;
		return *this ;
	}

// logical
	bool operator== ( VDate& rhs )
	{
		return y_ == rhs.y_ && m_ == rhs.m_ && d_ == rhs.d_ ;
	}

	bool operator! ()
	{
		return y_ == 0 || m_ == 0 || d_ == 0 ;
	}

// utilities
	static inline long isLeap( long year ) 
	{ 
		return year % 4 ? 0 : year % 100 ? 1 : year % 400 ? 0 : 1 ; 
	}
	
	static inline long leapDays( long year ) 
	{ 
		return ( year / 4 - year / 100 + year / 400 ) ; 
	}

	static inline bool isValid( long yr, long mo, long dy )
	{
		VDATE_DAY_V()

		return !( yr < 1 || mo < 1 || mo > 12 || dy < 1 
		|| dy > (mo == 2 ? 28 + isLeap( yr ) : ds[(mo + 9) % 12]) ) ;
	}

	static inline long rawCount( long yr, long mo, long da ) 
	{
		VDATE_COUNT_V()

		return  mo < 0 || mo > 12 ? 0 : 
				( yr * 365 
				+ leapDays( yr ) - ( mo < 3 ? isLeap( yr ) : 0 ) 
				+ days[mo] 
				+ da 
				- VDATE_MAGIC ) ;
	}
	
	static inline long qAtol( char *data, size_t len )
	{
		long result = (long)TO_DIGIT(*data) ;
		while ( --len ) {
			data++ ;
			result *= 10 ;
			result += (long)TO_DIGIT(*data) ;
		}
		return result ;
	}

	static inline bool parseYMD( char* date, long* yr, long* mo, long *dy )
	{

		*yr = qAtol( date, 4 ) ;
		*mo = qAtol( date + 4, 2 ) ;
		*dy = qAtol( date + 6, 2 ) ;
		return isValid( *yr, *mo, *dy ) ;
	}

// output
	double variantDate() 
	{
		return (double) (datum() - VDATE_ZERO) ;
	}

	long datum( )
	{
		return rawCount( y_ - VDATE_OFFSET, m_, d_ ) ;
	}
	
	long dow( ) // Sunday == 0 
	{
		return ((datum() + 2) % 7) ;
	}

	size_t y4md( char* buf )
	{
		return sprintf( buf, "%4d%02d%02d", y_, m_, d_ ) ;
	}

	size_t dmy_us( char* buf )
	{
		return sprintf( buf, "%2d/%2d/%4d", m_, d_, y_ ) ;
	}

	size_t dmy_us0( char* buf )
	{
		return sprintf( buf, "%02d/%02d/%4d", m_, d_, y_ ) ;
	}

	size_t dmy_eu( char* buf )
	{
		return sprintf( buf, "%2d/%2d/%4d", d_, m_, y_ ) ;
	}

	size_t dmy_eu0( char* buf )
	{
		return sprintf( buf, "%02d/%02d/%4d", d_, m_, y_ ) ;
	}

	size_t litmo( char* buf )
	{
		VDATE_MONTH_V()

		return sprintf( buf, "%2d-%s-%4d", d_, months[m_], y_ ) ;
	}

	size_t iso( char* buf )
	{
		return sprintf( buf, "%4d-%02d-%02d", y_, m_, d_ ) ;
	}

	char* format( enum style s )
	{
		if ( f_ != s ) {
			switch ( s ) {
			case DMY_US:	l_ = dmy_us( b_ ) ;   break ;
			case DMY_US0:	l_ = dmy_us0( b_ ) ;  break ;
			case DMY_EU:	l_ = dmy_eu( b_ ) ;   break ;
			case DMY_EU0:	l_ = dmy_eu0( b_ ) ;  break ;
			case LITMO:		l_ = litmo( b_ ) ;     break ;
			case ISO:		l_ = iso( b_ ) ;      break ;
			case Y4MD: 
			default:		l_ = y4md( b_ ) ;     break ;
			}
			f_ = s ;
		}
		return b_ ;
	}

protected:

	void parseDatum( long datum )
	{  
		VDATE_DAY_V()

		y_ = datum / 365  ;
		d_ = datum % 365 - leapDays( y_ ) ;

		if ( d_ > 0 ) {
			m_ = 0 ;
			while ( d_ > ds[m_] )
				d_ -= ds[m_++] ;
			m_ += 3 ;
			if ( m_ > 12 ) {
				m_ -= 12 ;
				y_++ ;
			}
		}
		else {
			d_ += isLeap( y_ ) ;
			m_ = 12 ;
			do {
				d_ += ds[--m_] ;
			} while ( d_ <= 0 ) ;
			m_ += 3 ;
			if ( m_ > 12 ) 
				m_ -= 12 ;
			else
				y_-- ;
		}
	}

} ;

#endif

/* ==[ NOTES ]================================================ +

  This is not completely general, for performance reasons.  
  It would be if parseDatum were invoked only on datum % 146097 
  (== days in 400 years), adjusted to be positive, e.g.

     long epoch = rawDatum / 146097 ;
	 long datum = rawDatum % 146097 ;
	 if ( datum < 0 ) { datum += 146097 ; epoch-- ; }
	 parseDatum( datum ) ;
	 y_ += 400 * epoch ;

     and finally, as usual,
	 y_ += DATUM_BASE_YEAR ;

  where DATUM_BASE_YEAR % 400 == 0

  Choosing a base year 0 mod 400 makes the math in isLeap() and 
  leapDays() completely regular and efficient.  
  
  We choose DATUM_BASE_YEAR as 1600, rather than 2000, because of 
  practicality: we need to handle dates in the (late) 1900's.

  The algorithm is based on treating 1-Mar-DATUM_BASE_YEAR == 1,
  in turn based on treating logical years as ending on the last
  day of February of the same calendar year. Thus the initial
  value of d_ will always be logical 0 for this particular day, 
  in terms of logical years.  We catch the leap year case neatly 
  in the else-branch of the final adjustment (the only way to 
  get Feb 29 is to have d_ == 0 and isLeap(y_) == 1.) 
  
 
  Factoid: 146097 % 7 == 0.  That is, the leap year cycle over
  400 years also spans exactly 20871 weeks.  Hmm...

 + =========================================================== */
