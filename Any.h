
#ifndef MY_ANY_H
#define MY_ANY_H

#include <typeinfo>
#include <utility>

    /**
     * @file: Any.h
     * @brief: Minimal implementation of std::any for C++11.
     * Does not have small object optimization.
     */

    class Any
    {
    public:
        ~Any() noexcept { destroy_( data_ ); }

        Any()
        : data_(nullptr)
        , destroy_([]( void* ) -> void {})
        , clone_([]( void * ) -> void* { return nullptr; })
        , type_([]() -> std::type_info const& { return typeid( nullptr ); })
        {}

        template<typename VT>  // VT = Value Type
        explicit Any(VT&& value)
        : data_(new VT(std::forward<VT>(value)))
        , destroy_([]( void* data ) -> void { delete static_cast<VT*>(data); })
        , clone_([]( void* data ) -> void* { return new VT(*static_cast<VT*>(data)); })
        , type_([]() -> std::type_info const& { return typeid( VT );})
        {}

        Any(Any const& model)
        : data_(model.clone_( model.data_ ))
        , destroy_(model.destroy_)
        , clone_(model.clone_)
        , type_(model.type_)
        {}

        Any(Any&& model)
        : data_(std::move(model.data_))
        , destroy_(std::move(model.destroy_))
        , clone_(std::move(model.clone_))
        , type_(std::move(model.type_))
        {}

        std::type_info const& type() const { return type_(); }
        void* data() const { return data_; }
        void swap( Any& rhs ) noexcept
        {
            using std::swap;
            swap( data_, rhs.data_ );
            swap( destroy_, rhs.destroy_ );
            swap( clone_, rhs.clone_ );           
            swap( type_, rhs.type_ );
        }

        Any& operator=( Any const& rhs )
        {
            Any(rhs).swap( *this );
            return *this;
        }

        Any& operator=( Any&& rhs )
        {
            rhs.swap( *this );
            return *this;
        }

        constexpr bool has_value() const noexcept { return data_; }

    private:
        void*                   data_;
        void                    (*destroy_)( void* );
        void*                   (*clone_)( void* );
        std::type_info const&   (*type_)();
    };

    struct BadAny
    : public std::bad_cast
    {
        char const* what() const noexcept { return "Bad cast of Any type"; }
    };
    
    template<typename VT>
    VT& any_cast( Any& any )
    {
        if ( typeid(VT) == any.type() )
        {
            return *static_cast<VT*>(any.data());
        }
        else { throw BadAny(); }
    }

    template<typename VT>
    VT const& any_cast( Any const& any )
    {
        if ( typeid(VT) == any.type() )
        {
            return *static_cast<const VT*>(any.data());
        }
        else { throw BadAny(); }
    }

    template<typename VT>
    VT* any_cast( Any* any )
    {
        if ( typeid(VT) == any->type() )
        {
            return static_cast<VT*>(any->data());
        }
        else { throw BadAny(); }
    }

    template<typename VT>
    VT const* any_cast( Any const* any )
    {
        if ( typeid(VT) == any->type() )
        {
            return static_cast<VT const*>(any->data());
        }
        else { throw BadAny(); }
    }

#endif // MY_ANY_H
