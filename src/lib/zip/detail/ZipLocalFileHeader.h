#pragma once

#include "ZipGenericExtraField.h"

#include <iostream>
#include <vector>
#include <cstdint>

class ZipArchiveEntry;

namespace detail {
    
    struct ZipCentralDirectoryFileHeader;
    
    struct ZipLocalFileHeaderBase : Serializable<ZipLocalFileHeaderBase> {
        
        u32 signature;
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
        
        u8 padding[2];
        
    };
    
    struct ZipLocalFileHeader: ZipLocalFileHeaderBase {
        
        struct constants {
    
            static constexpr u32 signature = 0x04034b50;
            static constexpr u32 dataDescriptorSignature = 0x08074b50;
            
        };
        
        std::string fileName;
        std::vector<ZipGenericExtraField> extraFields;
        
        ZipLocalFileHeader();
        
        void syncWithCentralDirectoryFileHeader(const ZipCentralDirectoryFileHeader& centralDirectoryFileHeader);
        
        bool deserialize(std::istream& stream);
        
        void serialize(std::ostream& stream) const;
        
        void deserializeAsDataDescriptor(std::istream& stream);
        
        void serializeAsDataDescriptor(std::ostream& stream) const;
        
    };
    
}
