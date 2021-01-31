
#include "Hsm.h"
#include <stdio.h>

    /**
     * @file hsmtst.cpp
     * @brief Demo of HSM implementation a la Heinzmann.
     * This is the Samek-Montgomery example state machine.
     *
     * Two versions, with or without CRTP, will compile.
     */

    enum Signal { A_SIG,B_SIG,C_SIG,D_SIG,
                  E_SIG,F_SIG,G_SIG,H_SIG };

using namespace hsm;

    // state machine
    class Host
#ifdef USE_CRTP
    : public Dispatcher<Host>
#endif
    {
    public:
        Host(); // needs concrete definitions of states
        ~Host() {}
        //
#ifdef USE_CRTP
        bool onSig( Signal sig ) { sig_ = sig; return dispatch(); }
#else
        void activate( TopState<Host> const& state ) { disp_.activate( state ); }
        bool onSig( Signal sig ) { sig_ = sig; return disp_.dispatch( *this ); }
#endif
        Signal getSig() const { return sig_; }
        //
        void foo( int i ) { foo_ = i; }
        int foo() const { return foo_; }

    private:
#ifndef USE_CRTP
        Dispatcher<Host> disp_;
#endif
        Signal    sig_;
        int       foo_;
    };

namespace hsm
{
    // states, must be defined in namespace hsm for template specializations to work
    using Top = CompState<Host, 0>;
    using   S0 = CompState<Host, 1, Top>;
    using     S1 = CompState<Host, 2, S0>;
    using       S11 = LeafState<Host, 3, S1>;
    using     S2 = CompState<Host, 4, S0>;
    using       S21 = CompState<Host, 5, S2>;
    using         S211 = LeafState<Host, 6, S21>;

    // init actions (note the reverse ordering!)
    template<> inline void S21::init( Host& h ) { Init<S211> i(h); printf("s21-INIT;"); }
    template<> inline void S2::init( Host& h ) { Init<S21> i(h); printf("s2-INIT;"); }
    template<> inline void S1::init( Host& h ) { Init<S11> i(h); printf("s1-INIT;"); }
    template<> inline void S0::init( Host& h ) { Init<S1> i(h); printf("s0-INIT;"); }
    template<> inline void Top::init( Host& h ) { Init<S0> i(h); printf("Top-INIT;"); }

    // entry actions
    template<> inline void S0::entry( Host& ) { printf("s0-ENTRY;"); }
    template<> inline void S1::entry( Host& ) { printf("s1-ENTRY;"); }
    template<> inline void S11::entry( Host& ) { printf("s11-ENTRY;"); }
    template<> inline void S2::entry( Host& ) { printf("s2-ENTRY;"); }
    template<> inline void S21::entry( Host& ) { printf("s21-ENTRY;"); }
    template<> inline void S211::entry( Host& ) { printf("s211-ENTRY;"); }

    // exit actions
    template<> inline void S0::exit( Host& ) { printf("s0-EXIT;"); }
    template<> inline void S1::exit( Host& ) { printf("s1-EXIT;"); }
    template<> inline void S11::exit( Host& ) { printf("s11-EXIT;"); }
    template<> inline void S2::exit( Host& ) { printf("s2-EXIT;"); }
    template<> inline void S21::exit( Host& ) { printf("s21-EXIT;"); }
    template<> inline void S211::exit( Host& ) { printf("s211-EXIT;"); }

} // namespace hsm

    Host::Host() { Top::init( *this); }

    template<>
    template<typename X> inline bool
    S0::handle( Host& h, const X& x ) const
    {
        switch ( h.getSig() )
        {
        case E_SIG: { Tran<X, This, S211> t(h); printf("s0-E;"); return true; }
        default: break;
        }
        return Base::handle( h, x );
    }

    template<>
    template<typename X> inline bool
    S1::handle( Host& h, const X& x ) const
    {
        switch( h.getSig() )
        {
        case A_SIG: { Tran<X, This, S1> t(h); printf("s1-A;"); return true; }
        case B_SIG: { Tran<X, This, S11> t(h); printf("s1-B;"); return true; }
        case C_SIG: { Tran<X, This, S2> t(h); printf("s1-C;"); return true; }
        case D_SIG: { Tran<X, This, S0> t(h); printf("s1-D;"); return true; }
        case F_SIG: { Tran<X, This, S211> t(h); printf("s1-F;"); return true; }
        default: break;
        }
        return Base::handle( h, x );
    }

    template<>
    template<typename X> inline bool
    S11::handle( Host& h, const X& x ) const
    {
        switch( h.getSig() )
        {
        case G_SIG: { Tran<X, This, S211> t(h); printf("s11-G;"); return true; }
        case H_SIG: if(h.foo()) { printf("s11-H;"); h.foo( 0 ); return true;
                    } break;
        default: break;
        }
        return Base::handle( h, x );
    }

    template<>
    template<typename X> inline bool
    S2::handle( Host& h, const X& x ) const
    {
        switch( h.getSig() )
        {
        case C_SIG: { Tran<X, This, S1> t(h); printf("s2-C;"); return true; }
        case F_SIG: { Tran<X, This, S11> t(h); printf("s2-F;"); return true; }
        default: break;
        }
        return Base::handle( h, x );
    }
 
    template<>
    template<typename X> inline bool
    S21::handle( Host& h, const X& x ) const
    {
        switch( h.getSig() )
        {
        case B_SIG: { Tran<X, This, S211> t(h); printf("s21-B;"); return true; }
        case H_SIG: if( !h.foo() ) { Tran<X, This, S21> t(h); printf("s21-H;"); h.foo( 1 ); return true;
                    } break;
        default: break;
        }
        return Base::handle( h, x );
    }

    template<>
    template<typename X> inline bool
    S211::handle( Host& h, const X& x ) const
    {
        switch( h.getSig() )
        {
        case D_SIG: { Tran<X, This, S21> t(h); printf("s211-D;"); return true; }
        case G_SIG: { Tran<X, This, S0> t(h); printf("s211-G;"); return true; }
        default: break;
        }
        return Base::handle( h, x );
    }


    int main()
    {
        Host test;
        for(;;)
        {
            printf("\nSignal<-");
            char c = getc( stdin );
            getc( stdin ); // discard '\n'
            if( c < 'a' || 'h' < c ) { break; }
            if ( !test.onSig( (Signal)(c-'a') ) )
                printf( "[Discarded!]" );
        }
        return 0;
    }
