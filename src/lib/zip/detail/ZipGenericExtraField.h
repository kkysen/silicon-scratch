#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "src/main/util/numbers.h"

namespace detail {
    
    struct ZipGenericExtraField {
        
        struct __attribute__((packed)) Header {
            u16 tag;
            u16 size;
        } header;
        
        std::vector<u8> data;
        
        u16 size() const noexcept;
        
        bool deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd);
        
        void serialize(std::ostream& stream);
        
    };
    
}
