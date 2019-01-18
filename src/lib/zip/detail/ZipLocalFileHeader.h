#pragma once

#include "ZipGenericExtraField.h"

#include <iostream>
#include <vector>
#include <cstdint>

class ZipArchiveEntry;

namespace detail {
    
    struct ZipCentralDirectoryFileHeader;
    
    struct ZipLocalFileHeaderBase {
        
        u32 signature;
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
        
        void deserializeBase(std::istream& stream);
        
        void serializeBase(std::ostream& stream);
        
    };
    
    struct ZipLocalFileHeader: ZipLocalFileHeaderBase {
        
        static constexpr u32 SIGNATURE_CONST = 0x04034b50;
        static constexpr u32 DATA_DESCRIPTOR_SIGNATURE = 0x08074b50;
        
        std::string fileName;
        std::vector<ZipGenericExtraField> extraFields;
        
        ZipLocalFileHeader();
        
        void syncWithCentralDirectoryFileHeader(const ZipCentralDirectoryFileHeader& centralDirectoryFileHeader);
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream);
        
        void deserializeAsDataDescriptor(std::istream& stream);
        
        void serializeAsDataDescriptor(std::ostream& stream);
        
    };
    
}
