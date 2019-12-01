#pragma once

#include <stdint.h>
#include "slice.h"

namespace common {

class Bits {
public:
    template<typename T>
    static inline int ones_count(T x) {
        int cnt = 0;
        while (x) {
            cnt += x & 0x01;
            x >>= 1;
        }

        return cnt;
    }

    static inline uint8_t get_bw4(const Slice& str, int i) {
        uint8_t word = str[i>>1];
        return i & 0x01 ? word & 15 : word >> 4;
    }

};

} // end of namespace common 
