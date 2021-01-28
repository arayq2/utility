#pragma once

#ifndef UTILITY_HSM_HEINZMANN_H
#define UTILITY_HSM_HEINZMANN_H

    /**
     * @file Hsm.h
     * @brief HSM implementation by Stefan Heinzmann.
     */

    // template arg: concrete machine (host)
    template<typename H>
    struct TopState
    {
        using Base = void;
        using Host = H;
        //
        virtual void handler( Host& ) const = 0;
        virtual unsigned getId() const = 0;
    };

    template<typename H, unsigned id, typename BaseState>
    struct CompState;

    template <typename H>
    struct CompState<H, 0, TopState<H> >
    : TopState<H>
    {
        using Base = TopState<H>;
        using This = CompState<H, 0, Base>;
        //
        template <typename X>
        void handle( H&, const X& ) const {} // discard event
        //
        static void init( H& ); // no default implementation
        static void entry( H& ) {}
        static void exit( H& ) {}
    };

    template<typename H, unsigned id, typename BaseState = CompState<H, 0, TopState<H> > >
    struct CompState
    : BaseState
    {
      using Base = BaseState;
      using This = CompState<H, id, Base>;
      //
      template<typename X>
      void handle( H& h, const X& x ) const { Base::handle( h, x ); }
      //
      static void init( H& ); // no default implementation
      static void entry( H& ) {}
      static void exit( H& ) {}
    };

    template<typename H, unsigned id, typename BaseState = CompState<H, 0, TopState<H> > >
    struct LeafState
    : BaseState
    {
      using Host = H;
      using Base = BaseState;
      using This = LeafState<H,id,Base>;
      //
      template<typename X>
      void handle( H& h, const X& x ) const { Base::handle( h, x ); }
      //
      virtual void handler( H& h ) const override { handle( h, *this ); }
      virtual unsigned getId() const override { return id; }
      //
      static void init( H& h ) { h.next( obj ); } // don't specialize this
      static void entry( H& ) {}
      static void exit( H& ) {}

      static LeafState const obj;
    };

    template<typename H, unsigned id, typename BaseState> 
    LeafState<H, id, BaseState> const LeafState<H, id, BaseState>::obj;

    template<class D, class B>
    class IsDerivedFrom
    {
        class Yes { char a[1]; };
        class No { char a[10]; };
        static Yes Test( B* ); // undefined
        static No Test( ... ); // undefined
    public:
        enum { Res = sizeof(Test(static_cast<D*>(0))) == sizeof(Yes) ? 1 : 0 };
    };
    
    template<bool> struct Bool{};

    template<typename C, typename S, typename T> // Current, Source, Target
    struct Tran
    {
        using Host = typename C::Host;
        using CurrentBase = typename C::Base;
        using SourceBase = typename S::Base;
        using TargetBase = typename T::Base;
        //
        enum { // work out when to terminate template recursion
            eTB_CB    = IsDerivedFrom<TargetBase, CurrentBase>::Res,
            eS_CB     = IsDerivedFrom<S, CurrentBase>::Res,
            eS_C      = IsDerivedFrom<S, C>::Res,
            eC_S      = IsDerivedFrom<C, S>::Res,
            exitStop  = eTB_CB && eS_C,
            entryStop = eS_C || eS_CB && !eC_S
        };
        // We use overloading to stop recursion. The
        // more natural template specialization
        // method would require to specialize the
        // inner template without specializing the
        // outer one, which is forbidden.
        static void exitActions( Host&, Bool<true> ) {}
        static void exitActions( Host& h, Bool<false> )
        {
            C::exit( h );
            Tran<CurrentBase, S, T>::exitActions( h, Bool<exitStop>() );
        }
        //
        static void entryActions( Host&, Bool<true> ) {}
        static void entryActions( Host& h, Bool<false> )
        {
            Tran<CurrentBase, S, T>::entryActions( h, Bool<entryStop>() );
            C::entry( h );
        }
        //
        Tran(Host& h)
        : host_(h)
        {
            exitActions( host_, Bool<false>() );
        }
        ~Tran()
        {
            Tran<T,S,T>::entryActions( host_, Bool<false>() );
            T::init( host_ );
        }
        //
        Host& host_;
    };

    template<typename T>
    struct Init
    {
        using Host = typename T::Host;
        //
        Init(Host& h) : host_(h) {}
        ~Init() { T::entry( host_ ); T::init( host_ ); }
        //
        Host& host_;
    };

#endif // UTILITY_HSM_HEINZMANN_H
