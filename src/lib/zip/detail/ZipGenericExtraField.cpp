#include "ZipGenericExtraField.h"

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    u16 ZipGenericExtraField::size() const noexcept {
        return sizeof(header) + data.size();
    }
    
    bool ZipGenericExtraField::deserialize(std::istream& stream, std::istream::pos_type extraFieldEnd) {
        if ((extraFieldEnd - stream.tellg()) < static_cast<std::istream::pos_type>(sizeof(header))) {
            return false;
        }
        
        // TODO are the :: supposed to be there?
        ::deserialize(stream, header);
        
        if ((extraFieldEnd - stream.tellg()) < header.size) {
            return false;
        }
        
        ::deserialize(stream, data, header.size);
        
        return true;
    }
    
    void ZipGenericExtraField::serialize(std::ostream& stream) {
        header.size = static_cast<u16>(data.size());
        ::serialize(stream, header);
        ::serialize(stream, data);
    }
    
}
