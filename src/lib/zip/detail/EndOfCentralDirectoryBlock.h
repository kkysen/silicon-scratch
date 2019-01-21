#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "src/main/util/numbers.h"
#include "src/lib/zip/streams/Serializable.h"

class ZipArchive;

class ZipArchiveEntry;

namespace detail {
    
    struct EndOfCentralDirectoryBlockBase : Serializable<EndOfCentralDirectoryBlockBase> {
        
        u32 signature;
        u16 diskNumber;
        u16 startOfCentralDirectoryDiskNum;
        u16 numberEntriesInDiskCentralDirectory;
        u16 numberEntriesInCentralDirectory;
        u32 centralDirectorySize;
        u32 offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber;
        mutable u16 commentLength;
        
        u8 padding[2];
        
    };
    
    struct EndOfCentralDirectoryBlock : EndOfCentralDirectoryBlockBase {
        
        struct constants {
    
            static constexpr u32 signature = 0x06054b50;
            
        };
        
        std::string comment;
        
        EndOfCentralDirectoryBlock();
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream) const;
        
    };
    
}
