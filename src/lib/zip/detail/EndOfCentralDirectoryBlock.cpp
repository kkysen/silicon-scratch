#include "EndOfCentralDirectoryBlock.h"
#include "../streams/serialization.h"
#include <cstring>

namespace detail {
    
    EndOfCentralDirectoryBlock::EndOfCentralDirectoryBlock() : EndOfCentralDirectoryBlockBase({}) {
        signature = constants::signature;
    }
    
    bool EndOfCentralDirectoryBlock::deserialize(std::istream& stream) {
        deserializeBase(stream);
        ::deserialize(stream, comment, commentLength);
        return true;
    }
    
    void EndOfCentralDirectoryBlock::serialize(std::ostream& stream) const {
        commentLength = static_cast<u16>(comment.length());
        serializeBase(stream);
        ::serialize(stream, comment);
    }
    
}
