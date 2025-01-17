/** ======================================================================+
 + Copyright @2023-2024 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef UTILITY_NOTICE_H
#define UTILITY_NOTICE_H
#pragma once

#include <future>

namespace Utility
{

    class Notice
    {
        using Promise = std::promise<void>;
        using Future  = std::future<void>;

    public:
        ~Notice() = default;
        Notice() = default;

        void reset()
        {
            promise_ = Promise();
        }

        void wait()
        {
            Future  _future(promise_.get_future());
            _future.wait();
            return _future.get();
        }

        void deliver()
        {
            promise_.set_value();
        }

    private:
        Promise     promise_;
    };


} // namespace Utility

#endif // UTILITY_NOTICE_H

