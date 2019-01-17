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
    
    struct __attribute__((packed)) ZipCentralDirectoryFileHeaderBase {
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
        u16 fileNameLength;
        u16 extraFieldLength;
        u16 fileCommentLength;
        u16 diskNumberStart;
        u16 internalFileAttributes;
        u32 externalFileAttributes;
        i32 relativeOffsetOfLocalHeader;
    };
    
    struct ZipCentralDirectoryFileHeader: ZipCentralDirectoryFileHeaderBase {
        
        static constexpr u32 SIGNATURE_CONSTANT = 0x02014b50;
        
        std::string fileName;
        std::vector<ZipGenericExtraField> extraFields;
        std::string fileComment;
        
        ZipCentralDirectoryFileHeader();
        
        void syncWithLocalFileHeader(const ZipLocalFileHeader& localFileHeader);
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream);
        
    };
    
}
