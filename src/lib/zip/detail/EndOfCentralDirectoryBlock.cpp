#include "EndOfCentralDirectoryBlock.h"
#include "../streams/serialization.h"
#include <cstring>

namespace detail {
    
    EndOfCentralDirectoryBlock::EndOfCentralDirectoryBlock() : EndOfCentralDirectoryBlockBase({}) {
        signature = SIGNATURE_CONST;
    }
    
    bool EndOfCentralDirectoryBlock::deserialize(std::istream& stream) {
        ::deserialize<EndOfCentralDirectoryBlockBase>(stream, *this);
        ::deserialize(stream, comment, commentLength);
        return true;
    }
    
    void EndOfCentralDirectoryBlock::serialize(std::ostream& stream) {
        commentLength = static_cast<uint16_t>(comment.length());
        ::serialize<EndOfCentralDirectoryBlockBase>(stream, *this);
        ::serialize(stream, comment);
    }
    
}
