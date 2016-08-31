#pragma once

#include <algorithm>
#include "log.h"

namespace agora { namespace base {

    struct pair_hasher
    {
        size_t operator() (const std::pair<uint32_t, uint32_t> &a) const {
            union {
                uint64_t n;
                struct {
                    uint32_t high;
                    uint32_t low;
                } bits;
            };

            bits.high = a.first;
            bits.low = a.second;
            return hasher_(n);
        }
    private:
        std::hash<uint64_t> hasher_;
    };

    template<class InputIterator, class OutputIterator, class UnaryFunction, class Predicate>
    OutputIterator transform_if(InputIterator first, InputIterator last, OutputIterator result, UnaryFunction f, Predicate pred)
    {
        for (; first != last; ++first) {
            if( pred(*first) )
                *result++ = f(*first);
        }
        return result;
    }

    template<typename _Container, typename _Fn1>
    inline void retain(_Container & _C, _Fn1 _Func)
    {
        typename _Container::iterator it = _C.begin();
        while (it != _C.end()) {
            if (_Func(*it)) {
                ++it;
            } else {
                it = _C.erase(it);
            }
        }
    }

    template<typename _Container, typename _Fn1>
    inline void retain_till(_Container & _C, _Fn1 _Func)
    {
        typename _Container::iterator it = _C.begin();
        while (it != _C.end()) {
            if (_Func(*it)) {
                return ;
            }
            it = _C.erase(it);
        }
    }

    template<typename ElementType, typename _Fn1>
    inline void retain_till(std::vector<ElementType> & _C, _Fn1 _Func)
    {
        _C.erase(_C.begin(), std::find_if(_C.begin(), _C.end(), _Func));
    }

    template<typename ElementType>
    inline ElementType average_least(const std::vector<ElementType> & numbers, size_t least)
    {
        if (numbers.empty()) {
            return 0;
        }

        std::vector<ElementType> sorted = numbers;
        std::sort(sorted.begin(), sorted.end());
        ElementType total = 0;
        size_t count = std::min(least, sorted.size());
        for (size_t i = 0; i < count; ++i) {
            total += sorted[i];
        }
        return total / count;
    }
}}
