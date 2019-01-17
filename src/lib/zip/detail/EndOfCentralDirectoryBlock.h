#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "src/main/util/numbers.h"

class ZipArchive;

class ZipArchiveEntry;

namespace detail {
    
    struct __attribute__((packed)) EndOfCentralDirectoryBlockBase {
        u32 signature;
        u16 diskNumber;
        u16 startOfCentralDirectoryDiskNum;
        u16 numberEntriesInDiskCentralDirectory;
        u16 numberEntriesInCentralDirectory;
        u32 centralDirectorySize;
        u32 offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber;
        u16 commentLength;
    };
    
    struct EndOfCentralDirectoryBlock : EndOfCentralDirectoryBlockBase {
        
        static constexpr u32 SIGNATURE_CONST = 0x06054b50;
        
        std::string comment;
        
        EndOfCentralDirectoryBlock();
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream);
        
    };
    
}
