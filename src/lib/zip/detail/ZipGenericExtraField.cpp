#include "ZipGenericExtraField.h"

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    u16 ZipGenericExtraField::size() const noexcept {
        return static_cast<u16>(Header::Serializable::size + data.size());
    }
    
    bool ZipGenericExtraField::deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd) {
        if ((extraFieldEnd - stream.tellg()) < static_cast<std::istream::pos_type>(sizeof(header))) {
            return false;
        }
        
        header.deserializeBase(stream);
        
        if ((extraFieldEnd - stream.tellg()) < header.size) {
            return false;
        }
        
        ::deserialize(stream, data, header.size);
        
        return true;
    }
    
    void ZipGenericExtraField::serialize(std::ostream& stream) const {
        header.size = static_cast<u16>(data.size());
        header.serializeBase(stream);
        ::serialize(stream, data);
    }
    
}
