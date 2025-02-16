/** ======================================================================+
 + Copyright @2020 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_FINALLY_H
#define UTILITY_FINALLY_H

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

        void cancel() { action_ = [](){}; }

        ~Finally() noexcept
        {
            try { action_(); } catch (...) {}
        }

    private:
        Action  action_;
    };

} // namespace Utility

#endif // UTILITY_FINALLY_H
