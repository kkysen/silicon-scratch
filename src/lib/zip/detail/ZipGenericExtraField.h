#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "src/main/util/numbers.h"
#include "src/lib/zip/streams/Serializable.h"

namespace detail {
    
    struct ZipGenericExtraField {
        
        struct Header : Serializable<Header> {
            
            u16 tag;
            mutable u16 size;
    
            u8 padding[0];
            
        } header;
        
        std::vector<u8> data;
        
        u16 size() const noexcept;
        
        bool deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd);
        
        void serialize(std::ostream& stream) const;
        
    };
    
}
