/** ======================================================================+
 + Copyright @2020-2021 Arjun Ray
 + Released under MIT License
 + see https://mit-license.org
 +========================================================================*/
#pragma once

#ifndef AMS_ENDPOINT_H
#define AMS_ENDPOINT_H

#include <string>
#include <utility>

namespace ams
{
    /**
     * @struct EndPoint
     * @brief Convenience class for destination strings
     *        that are topics by default
     */
    struct EndPoint
    {
        std::string     dest_;
        bool            isTopic_{false};

        ~EndPoint() = default;
        EndPoint() = default;

        //!> NOT explicit! Allow implicit conversions
        EndPoint(std::string const& dest, bool isQueue = false)
        : dest_(dest)
        , isTopic_(!isQueue) //!< note inversion
        {}

        EndPoint(std::string&& dest, bool isQueue = false)
        : dest_(std::move(dest))
        , isTopic_(!isQueue) //!< note inversion
        {}

        char const* str() const { return dest_.c_str(); }
        operator std::string() { return dest_; }

    };

} // namespace ams

#endif // AMS_ENDPOINT_H
