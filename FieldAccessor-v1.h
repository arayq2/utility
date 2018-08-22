#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>

/**
 * FieldAccessor.h
 * Mechanism to access the fields of a class in a generic fashion, using a
 * fairly standard C++ type erasure technique.  This is essentially the
 * functionality of std::function (without its full-generality overhead).
 * The basic context is flat-file processing, especially "wide" files (100+
 * fields), where the fields are associated with data members of a class.
 * See FieldInput.h and FieldOutput.h for two different examples of use.
 */
namespace Utility
{
    /**
     * This is the lowest level, where a field reference is associated with
     * some action, e.g. filling from a string, or writing a delimited string.
     * Field references are either unitary or in the context of a vector, these
     * (scalar, array) being the fundamental modalities of a class data member.
     * Custom fields (such as other sub-classes) are supported by specializing
     * the template and implementing the operator () entry points as needed.
     */
    template <typename Field, typename Functor>
    struct DefaultFunction
    {
        typedef Field    target_type;
        typedef Functor  argument_type;

        void operator () ( Field& field, Functor& functor ) const
        {
            functor( field );
        }

        void operator () ( boost::shared_ptr<Field>& field, Functor& functor ) const
        {
            Field        _dummy;
            functor( field ? *field : _dummy );
        }

        void operator () ( std::vector<Field>& fv, Functor& functor ) const
        {
            std::for_each( fv.begin(), fv.end(), functor );
        }

        template <size_t N>
        void operator () ( Field (&fp)[N], Functor& functor ) const
        {
            std::for_each( fp, fp + N, functor );
        }
    };


    /**
     * FieldAccessor: Uniform interface to accessors of appropriate internal types.
	 * A significantly shorter implementation is possible with a std::function member
	 * holding lambdas created by the constructors.
     */
    template<
        typename Object, 
        typename Argument, 
        template <
			typename Field, 
			typename Argument
			> class DefaultAction = DefaultFunction
        >
    class FieldAccessor
    {
    public:    
        // pack resolver and action wrapper functor of appropriate type
        FieldAccessor()
        : resolver_(new NoOp())
        {}
        
        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(Field Object::*fp, Action const& action = Action())
        : resolver_(new ScalarAccess<Field, Action>(fp, action))
        {}

        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(boost::shared_ptr<Field> Object::*fp, Action const& action = Action())
        : resolver_(new BoostAccess<Field, Action>(fp, action))
        {}

        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(std::vector<Field> Object::*fvp, Action const& action = Action())
        : resolver_(new VectorAccess<Field, Action>(fvp, action))
        {}

        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(size_t index, std::vector<Field> Object::*fvp, Action const& action = Action())
        : resolver_(new IndexAccess<Field, Action>(fvp, index, action))
        {}
        
        template<typename Field, size_t N, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(Field (Object::*fap)[N], Action const& action = Action())
        : resolver_(new ArrayAccess<Field, N, Action>(fap, action))
        {}
        
        template<typename Field, size_t N, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(size_t index, Field (Object::*fap)[N], Action const& action = Action())
        : resolver_(new SlotAccess<Field, N, Action>(fap, index, action))
        {}

        FieldAccessor(FieldAccessor const& model)
        : resolver_(model.resolver_->clone())
        {}

        ~FieldAccessor()
        {
            delete resolver_;
        }

        void swap( FieldAccessor& other )
        {
            std::swap( resolver_, other.resolver_ );
        }

        FieldAccessor& operator= ( FieldAccessor copy ) // assign == copy + swap
        {
            swap( copy );
            return *this;
        }

        // delegate work to packed functor
        void operator () ( Object& target, Argument& arg ) const
        {
            resolver_->invoke( target, arg );
        }
        
    private:
        struct Resolver; // type-erasing base class of resolvers
        Resolver    *resolver_;

        /**
         * This is the resolver level, where a data member pointer is resolved
         * with respect to an object instance for an actual field reference,
         * which can be passed on to the action function along with its argument.
         */

        struct Resolver
        {
            virtual ~Resolver() {}
            virtual void invoke( Object&, Argument& ) const = 0;
            virtual Resolver* clone() const = 0;
        };

        struct NoOp
        : public Resolver
        {
            NoOp() {}
            
            void invoke( Object&, Argument& ) const {}

            NoOp* clone() const
            {
                return new NoOp();
            }
        };
        
        template <typename Field, typename Action>
        struct ScalarAccess
        : public Resolver
        {
            Field Object::*            fp_;
            Action                     action_;
            
            ScalarAccess(Field Object::*fp, Action const& action)
            : fp_(fp)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                action_( target.*fp_, arg );
            }

            ScalarAccess* clone() const
            {
                return new ScalarAccess(fp_, action_);
            }
        };
    
        template <typename Field, typename Action>
        struct BoostAccess
        : public Resolver
        {
            boost::shared_ptr<Field> Object::*    fp_;
            Action                                action_;
            
            BoostAccess(boost::shared_ptr<Field> Object::*fp, Action const& action)
            : fp_(fp)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                if ( (target.*fp_) )
                {
                    action_( *(target.*fp_), arg ); 
                }
            }

            BoostAccess* clone() const
            {
                return new BoostAccess(fp_, action_);
            }
        };
    
        template <typename Field, typename Action>
        struct VectorAccess
        : public Resolver
        {
            std::vector<Field> Object::*    fvp_;
            Action                          action_;
            
            VectorAccess(std::vector<Field> Object::*fvp, Action const& action)
            : fvp_(fvp)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                action_( target.*fvp_, arg );
            }

            VectorAccess* clone() const
            {
                return new VectorAccess(fvp_, action_);
            }
        };
    
        template <typename Field, typename Action>
        struct IndexAccess
        : public Resolver
        {
            std::vector<Field> Object::*    fvp_;
            size_t                          index_;
            Action                          action_;
            
            IndexAccess(std::vector<Field> Object::*fvp, size_t index, Action const& action)
            : fvp_(fvp)
            , index_(index)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                action_( (target.*fvp_)[index_], arg );
            }

            IndexAccess* clone() const
            {
                return new IndexAccess(fvp_, index_, action_);
            }
        };
    
        template <typename Field, size_t N, typename Action>
        struct ArrayAccess
        : public Resolver
        {
            typedef Field (Object::*Pointer)[N];
            Pointer                        fap_;
            Action                         action_;
            
            ArrayAccess(Field (Object::*fap)[N], Action const& action)
            : fap_(fap)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                action_( target.*fap_, arg );
            }

            ArrayAccess* clone() const
            {
                return new ArrayAccess(fap_, action_);
            }
        };
    
        template <typename Field, size_t N, typename Action>
        struct SlotAccess
        : public Resolver
        {
            typedef Field (Object::*Pointer)[N];
            Pointer                        fap_;
            size_t                         index_;
            Action                         action_;
            
            SlotAccess(Field (Object::*fap)[N], size_t index, Action const& action)
            : fap_(fap)
            , index_(index)
            , action_(action)
            {}
            
            void invoke( Object& target, Argument& arg ) const
            {
                action_( (target.*fap_)[index_], arg );
            }

            SlotAccess* clone() const
            {
                return new SlotAccess(fap_, index_, action_);
            }
        };
    };

} // namespace Utility

