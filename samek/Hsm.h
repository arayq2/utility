#pragma once

#ifndef UTILITY_HSM_H
#define UTILITY_HSM_H

#include <map>
#include <cassert>

namespace Utility
{
    /**
     * @class Hsm
     * @brief Hierarchical State Machine (Samek-Montgomery).
     *
     * Abstract machine: header only, no implementation file.
     * Concrete machine: derives by CRTP.
     */
    template<typename Client, typename Message, std::size_t MAX_LEVELS = 8>
    class Hsm
    {
    public:
        using MsgPtr  = Message const*;
        // Messages internal to the machine logic
        struct SpecMsg 
        {
            MsgPtr  enterMsg_;
            MsgPtr  startMsg_;
            MsgPtr  leaveMsg_;
        };

    //=================================================================
        /**
         * @class State
         * @brief Helper class to wrap details of state-specific handlers.
         *
         * Client instantiates one for each state and specifies hierarchy
         * via the super_ pointer.
         * A special top state has no super-state, defaults all input,
         * and is the starting point of the machine.
         */
        class State
        {
        public:
            using Handler = MsgPtr (Client::*)( MsgPtr );
            //
            ~State() = default;
            State(char const* name, State* super, Handler handler)
            : super_(super)
            , name_(name)
            , handler_(handler)
            {}
            //
            State* super() const { return super_; }

        private:
            State*      super_;
            char const* name_;
            Handler     handler_;
            //
            MsgPtr on_event( Client* cxt, MsgPtr msg ) const
            {
                return (cxt->*handler_)( msg );
            }
            friend class Hsm;
        };

    //=================================================================
        /**
         * @class LcaCache
         * @brief Helper class to memoize Lowest Common Ancestor info.
         */
        class LcaCache
        {
            using FromTo = std::pair<State*, State*>;
            using Map    = std::map<FromTo, int>;
        public:
            ~LcaCache() = default;
            LcaCache() = default;
            //
            int lca( State* from, State* to ) 
            {
                FromTo  _pr{from, to};
                auto    _itr(map_.find( _pr ));
                if ( _itr != map_.end() ) { return _itr->second; }
                return map_[_pr] = compute_lca_( from, to );
            }

        private:
            Map     map_;
            //
            int compute_lca_( State* src, State* tgt )
            {
                if ( src == tgt ) { return 1; }
                int _steps{0};
                for ( State* _s = src; _s; ++_steps, _s = _s->super() )
                for ( State* _t = tgt; _t; _t = _t->super() )
                if ( _s == _t ) { return _steps; }
                return 0;
            }
        };

    //=================================================================
        ~Hsm() = default;
        Hsm(char const* name, State& top, SpecMsg& sm)
        : name_(name)
        , smp_(&sm)
        , topp_(&top)
        {}

        void on_start()
        {
            curp_ = topp_;
            nxtp_ = nullptr;
            curp_->on_event( THIS(), smp_->enterMsg_ );
            while ( curp_->on_event( THIS(), smp_->startMsg_ ), nxtp_ )
            {
                enter_();
            }
        }

        void dispatch( MsgPtr msg )
        {
            for ( State* _st = curp_; _st; _st = _st->super() )
            {
                srcp_ = _st;   // mark outermost handler for LCA
                msg = _st->on_event( THIS(), msg );
                if ( !msg )
                {   // message was processed
                    maybe_transit_();
                    break;
                }
            }
        }

    protected: // called from subclass (concrete machine)
        void STATE_START( State* target )
        {
            assert( nullptr == nxtp_ );
            nxtp_ = target;
        }

        void STATE_TRAN( State* target )
        {
            assert( nullptr == nxtp_ );
            leave_( cache_.lca( srcp_, target ) );
            nxtp_ = target;
        }

    private:
        char const* name_;
        SpecMsg*    smp_;
        State*      topp_;
        State*      srcp_{nullptr};
        State*      curp_{nullptr};
        State*      nxtp_{nullptr};
        LcaCache    cache_;

        // pre-transition: invoke exit processing
        void leave_( int steps )
        {   // fire leave msg for each state in super chain, bottom up
            State*  _st = curp_;
            //
            while ( _st != srcp_ )
            {
                _st->on_event( THIS(), smp_->leaveMsg_ );
                _st = _st->super();
            }
            while ( steps-- )
            {
                _st->on_event( THIS(), smp_->leaveMsg_ );
                _st = _st->super();
            }
            curp_ = _st;
        }

        // post-transition: invoke entry processing
        void enter_()
        {   // fire enter msg for each state in super chain, top down
            State*  _path[MAX_LEVELS];
            State** _trace = _path;
            *_trace = nullptr;
            //
            State*  _st;
            for ( _st = nxtp_; _st != curp_; _st = _st->super() )
            {
                *(++_trace) = _st;
            }
            while ( (_st = *_trace--) )
            {
                _st->on_event( THIS(), smp_->enterMsg_ );
            }
            curp_ = nxtp_;
            nxtp_ = nullptr;
        }

        // check for transition
        void maybe_transit_()
        {
            if ( nxtp_ )
            {
                do {
                    enter_();
                } while ( curp_->on_event( THIS(), smp_->startMsg_ ), nxtp_ );
            }
        }

        // safe down-cast due to CRTP
        Client* THIS() { return static_cast<Client*>(this); }
    };

} // namespace Utility

#endif // UTILITY_HSM_H
