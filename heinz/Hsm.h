#pragma once

#ifndef UTILITY_HSM_HEINZMANN_H
#define UTILITY_HSM_HEINZMANN_H

    /**
     * @file Hsm.h
     * @brief Hierarchical State Machine.
     * Based on a design using templates, by Stefan Heinzmann.
     * Overload, 12(64):, December 2004
     * @see https://accu.org/journals/overload/12/64/heinzmann_252/
     *
     * Key concepts:
     * (1) Each state is a separate type, with "init", "entry",
     *     and "exit" routines mapped to static methods.
     * (2) The event system processed by the state handlers
           is left to the concrete implementation.
     * (3) All event processing starts in a leaf state.
     * (4) The "init" event of a composite state must
           transition internally to a sub-state.
     * (5) The "init" event in a leaf state must set itself
     *     as the target of the next call to dispatch.
     */

    // template arg: concrete machine (host)
    template<typename H>
    struct TopState
    {
        using Host = H;
        using Base = void;
        // leaf states override:
        virtual bool handler( Host& ) const = 0;
        virtual unsigned getId() const = 0;
    };

    // CRTP: host (H parameter) derives from this
    template<typename H>
    class Dispatcher
    {
    public:
        void activate( TopState<H> const& s ) { state_ = &s; }
        bool dispatch() { return state_->handler( static_cast<H&>(*this) ); }

    private:
        TopState<H> const* state_{nullptr};
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
        bool handle( H&, const X& ) const { return false; } // discard event
        //
        static void init( H& ); // no default implementation, must define
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
      bool handle( H& h, const X& x ) const { return Base::handle( h, x ); }
      //
      static void init( H& ); // no default implementation, must define
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
      bool handle( H& h, const X& x ) const { return Base::handle( h, x ); }
      //
      virtual bool handler( H& h ) const override { return handle( h, *this ); }
      virtual unsigned getId() const override { return id; }
      //
      static void init( H& h ) { h.activate( obj ); } // do NOT specialize this!
      static void entry( H& ) {}
      static void exit( H& ) {}

      static LeafState const obj; // for dispatching
    };

    // instantiated with concrete machine
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

    template<bool> struct Bool{}; // discriminator for recursion

    template<typename Current, typename Source, typename Target>
    struct Tran
    {
        using Host        = typename Current::Host;
        using CurrentBase = typename Current::Base;
        using SourceBase  = typename Source::Base;
        using TargetBase  = typename Target::Base;
        //
        enum { // work out when to terminate template recursion
            eTB_CB    = IsDerivedFrom<TargetBase, CurrentBase>::Res,
            eS_CB     = IsDerivedFrom<Source, CurrentBase>::Res,
            eS_C      = IsDerivedFrom<Source, Current>::Res,
            eC_S      = IsDerivedFrom<Current, Source>::Res,
            exitStop  = eTB_CB && eS_C,
            entryStop = eS_C || eS_CB && !eC_S
        };
        // Overloading to terminate recursion.
        static void exitActions( Host&, Bool<true> ) {}
        static void exitActions( Host& h, Bool<false> )
        {
            Current::exit( h );
            Tran<CurrentBase, Source, Target>::exitActions( h, Bool<exitStop>() );
        }
        //
        static void entryActions( Host&, Bool<true> ) {}
        static void entryActions( Host& h, Bool<false> )
        {
            Tran<CurrentBase, Source, Target>::entryActions( h, Bool<entryStop>() );
            Current::entry( h );
        }
        // exit sequence in constructor, bottom up
        Tran(Host& h)
        : host_(h)
        {
            exitActions( host_, Bool<false>() );
        }
        // entry sequence in destructor, top down
        ~Tran()
        {
            Tran<Target,Source,Target>::entryActions( host_, Bool<false>() );
            Target::init( host_ );
        }
        //
        Host& host_;
    };

    template<typename State>
    struct Init
    {
        using Host = typename State::Host;
        //
        Init(Host& h) : host_(h) {}
        ~Init() { State::entry( host_ ); State::init( host_ ); }
        //
        Host& host_;
    };

#endif // UTILITY_HSM_HEINZMANN_H
