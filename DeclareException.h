#pragma once

#include <stdexcept>
#include <string>

    // This should be a nested class and thus it needs no distinct
    // name, as it will be namespaced by the containing class.
    //
#ifndef DECLARE_EXCEPTION
#define DECLARE_EXCEPTION \
    struct Exception : virtual public std::runtime_error\
    { Exception(std::string const& msg) : std::runtime_error(msg) {} }
#endif
#ifndef DECLARE_SUBEXCEPTION
#define DECLARE_SUBEXCEPTION(XX) \
    struct XX##Exception : virtual public std::runtime_error\
    { XX##Exception(std::string const& msg = #XX) : std::runtime_error(msg) {} }
#endif
