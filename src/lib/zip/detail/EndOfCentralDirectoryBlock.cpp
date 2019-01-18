#include "EndOfCentralDirectoryBlock.h"
#include "../streams/serialization.h"
#include <cstring>

namespace detail {
    
    EndOfCentralDirectoryBlock::EndOfCentralDirectoryBlock() : EndOfCentralDirectoryBlockBase({}) {
        signature = SIGNATURE_CONST;
    }
    
    void EndOfCentralDirectoryBlockBase::deserializeBase(std::istream& stream) {
        // packing the struct causes issues with references
//        ::deserialize(stream, *this);

        ::deserialize(stream, signature);
        ::deserialize(stream, diskNumber);
        ::deserialize(stream, startOfCentralDirectoryDiskNum);
        ::deserialize(stream, numberEntriesInDiskCentralDirectory);
        ::deserialize(stream, numberEntriesInCentralDirectory);
        ::deserialize(stream, centralDirectorySize);
        ::deserialize(stream, offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber);
        ::deserialize(stream, commentLength);
    }
    
    void EndOfCentralDirectoryBlockBase::serializeBase(std::ostream& stream) {
        // packing the struct causes issues with references
//        ::serialize(stream, *this);
    
        ::serialize(stream, signature);
        ::serialize(stream, diskNumber);
        ::serialize(stream, startOfCentralDirectoryDiskNum);
        ::serialize(stream, numberEntriesInDiskCentralDirectory);
        ::serialize(stream, numberEntriesInCentralDirectory);
        ::serialize(stream, centralDirectorySize);
        ::serialize(stream, offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber);
        ::serialize(stream, commentLength);
    }
    
    bool EndOfCentralDirectoryBlock::deserialize(std::istream& stream) {
        deserializeBase(stream);
        ::deserialize(stream, comment, commentLength);
        return true;
    }
    
    void EndOfCentralDirectoryBlock::serialize(std::ostream& stream) {
        commentLength = static_cast<u16>(comment.length());
        serializeBase(stream);
        ::serialize(stream, comment);
    }
    
}
