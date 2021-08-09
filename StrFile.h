#pragma once

#ifndef UTILITY_STRFILE_H
#define UTILITY_STRFILE_H

#include <utility>

namespace Utility
{
    /**
     * @class StrFile
     * @brief File into memory buffer
     */
    class StrFile
    {
    public:
        ~StrFile() noexcept;
        StrFile(char const* file);
        StrFile(StrFile&&); // move-only

        explicit operator bool() const { return base_ != nullptr; }
        int error() const { return err_; }

        using Spec = std::pair<char const*, std::size_t>;
        Spec spec() const { return {get(), size()}; }

        char const* get() const { return static_cast<char const*>(base_); }
        std::size_t size() const { return fsize_; }

    private:
        std::size_t fsize_{0};
        std::size_t msize_{0};
        void*       base_{nullptr};
        int         err_{0};
		//
		void map_normal_( int fd, std::size_t size );
		void map_special_( int fd, std::size_t size );
    };

    inline
    StrFile::StrFile(StrFile&& model)
    : fsize_(model.fsize_)
    , msize_(model.msize_)
    , base_(model.base_)
    , err_(model.err_)
    {
        model.base_ = nullptr;
    }
} // namespace Utility
#endif // UTILITY_STRFILE_H
