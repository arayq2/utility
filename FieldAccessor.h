#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

/**
 * FieldAccessor.h
 * Mechanism to access the fields of a class in a generic fashion.
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
     */
    template<
        typename Object, 
        typename Argument, 
        template <typename Field, typename Argument> class DefaultAction = DefaultFunction
        >
    class FieldAccessor
    {
    public:    
        // pack resolver and action wrapper functor of appropriate type
        FieldAccessor()
        : resolver_([]( Object&, Argument& ) {})
        {}
        
        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(Field Object::*fp, Action const& action = Action())
		: resolver_([=]( Object& obj, Argument& arg ) { action( obj.*fp, arg ); })
        {}

        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(std::vector<Field> Object::*fvp, Action const& action = Action())
		: resolver_([=]( Object& obj, Argument& arg ) { action( obj.*fvp, arg ); })
        {}

        template <typename Field, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(size_t index, std::vector<Field> Object::*fvp, Action const& action = Action())
		: resolver_([=]( Object& obj, Argument& arg ) { action( (obj.*fvp)[index], arg ); })
        {}
        
        template<typename Field, size_t N, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(Field (Object::*fap)[N], Action const& action = Action())
		: resolver_([=]( Object& obj, Argument& arg ) { action( obj.*fap, arg ); })
        {}
        
        template<typename Field, size_t N, typename Action = DefaultAction<Field, Argument> >
        FieldAccessor(size_t index, Field (Object::*fap)[N], Action const& action = Action())
		: resolver_([=]( Object& obj, Argument& arg ) { action( (obj.*fap)[index], arg ); })
        {}

        FieldAccessor(FieldAccessor const& model)
        : resolver_(model.resolver_)
        {}

        ~FieldAccessor() {}

        void swap( FieldAccessor& other )
        {
            resolver_.swap( other.resolver_ );
        }

        FieldAccessor& operator= ( FieldAccessor copy ) // assign == copy + swap
        {
            swap( copy );
            return *this;
        }

        // delegate work to packed functor
        void operator () ( Object& target, Argument& arg ) const
        {
            resolver_( target, arg );
        }
        
    private:
        std::function<void(Object&, Argument&)>    resolver_;

    };

} // namespace Utility

