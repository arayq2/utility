#pragma once

namespace Utility
{
    // IteratorRange.
    // Transformer to support C++ ranged-for syntax
    //
    template<typename Iterator>
    class IteratorRange
    {
    public:
        IteratorRange(Iterator begin, Iterator end)
        : begin_(begin)
        , end_(end)
        {}
        
        using iterator = Iterator;
        
        iterator begin() const { return begin_; }
        iterator end() const { return end_; }
        
    private:
        Iterator    begin_;
        Iterator    end_;
    };
    

} // namespace Utility
