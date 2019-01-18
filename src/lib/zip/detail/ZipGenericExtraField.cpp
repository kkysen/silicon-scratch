#include "ZipGenericExtraField.h"

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    void ZipGenericExtraField::Header::deserialize(std::istream& stream) {
        // packing the struct causes issues with references
//        ::deserialize(stream, *this);
    
        ::deserialize(stream, tag);
        ::deserialize(stream, size);
    }
    
    void ZipGenericExtraField::Header::serialize(std::ostream& stream) {
        // packing the struct causes issues with references
//        ::serialize(stream, *this);
    
        ::serialize(stream, tag);
        ::serialize(stream, size);
    }
    
    u16 ZipGenericExtraField::size() const noexcept {
        return sizeof(header) + data.size();
    }
    
    bool ZipGenericExtraField::deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd) {
        if ((extraFieldEnd - stream.tellg()) < static_cast<std::istream::pos_type>(sizeof(header))) {
            return false;
        }
        
        header.deserialize(stream);
        
        if ((extraFieldEnd - stream.tellg()) < header.size) {
            return false;
        }
        
        ::deserialize(stream, data, header.size);
        
        return true;
    }
    
    void ZipGenericExtraField::serialize(std::ostream& stream) {
        header.size = static_cast<u16>(data.size());
        header.serialize(stream);
        ::serialize(stream, data);
    }
    
}
