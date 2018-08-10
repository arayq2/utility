#pragma once

#include <functional> // std::function
#include <utility>    // std::move

namespace Utility
{
    /**
     * Finally.
     * Guaranteed action on exit from scope, unless cancelled. 
     * Useful for "undo" and "cleanup" functionality.
     */
    class Finally
    {
    public:
        using Action = std::function<void()>;

        explicit
        Finally(Action&& action)
        : action_(std::move(action))
        {}

        void cancel() { cancelled_ = true; }

        ~Finally() noexcept
        {
            if ( !cancelled_ ) try { action_(); } catch (...) {}
        }

    private:
        Action  action_;
        bool    cancelled_{false};
    };

} // namespace Utility
