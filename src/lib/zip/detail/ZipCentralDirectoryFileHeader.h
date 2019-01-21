#pragma once

#include "ZipGenericExtraField.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "src/main/util/numbers.h"

class ZipArchive;

class ZipArchiveEntry;

namespace detail {
    
    struct ZipLocalFileHeader;
    
    struct ZipCentralDirectoryFileHeaderBase1 {
        
        u32 signature;
        u16 versionMadeBy;
        u16 versionNeededToExtract;
        u16 generalPurposeBitFlag;
        u16 compressionMethod;
        u16 lastModificationTime;
        u16 lastModificationDate;
        u32 crc32;
        u32 compressedSize;
        u32 unCompressedSize;
        mutable u16 fileNameLength;
        mutable u16 extraFieldLength;
        mutable u16 fileCommentLength;
        u16 diskNumberStart;
        u16 internalFileAttributes;
        
        u8 padding1[2];
        
        void deserializeBase1(std::istream& stream);
        
        void serializeBase1(std::ostream& stream) const;
        
    };
    
    struct ZipCentralDirectoryFileHeaderBase2 {
        
        u32 externalFileAttributes;
        i32 relativeOffsetOfLocalHeader;
        
        u8 padding2[0];
    
        void deserializeBase2(std::istream& stream);
    
        void serializeBase2(std::ostream& stream) const;
        
    };
    
    struct ZipCentralDirectoryFileHeader
            : ZipCentralDirectoryFileHeaderBase1, ZipCentralDirectoryFileHeaderBase2 {
        
        struct constants {
    
            static constexpr u32 signature = 0x02014b50;
            
        };
        
        using Base1 = ZipCentralDirectoryFileHeaderBase1;
        using Base2 = ZipCentralDirectoryFileHeaderBase2;
        
        static constexpr size_t size =
                (sizeof(Base1) - sizeof(Base1().padding1))
                + (sizeof(Base2) - sizeof(Base2().padding2));
        
        std::string fileName;
        std::vector<ZipGenericExtraField> extraFields;
        std::string fileComment;
        
        ZipCentralDirectoryFileHeader();
        
        void syncWithLocalFileHeader(const ZipLocalFileHeader& localFileHeader);
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream) const;
        
    };
    
}
