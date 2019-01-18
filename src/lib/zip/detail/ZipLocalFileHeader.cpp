#include "ZipLocalFileHeader.h"
#include "ZipCentralDirectoryFileHeader.h"

#include <cstring>

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    ZipLocalFileHeader::ZipLocalFileHeader() : ZipLocalFileHeaderBase({}) {
        signature = SIGNATURE_CONST;
    }
    
    void ZipLocalFileHeader::syncWithCentralDirectoryFileHeader(
            const ZipCentralDirectoryFileHeader& centralDirectoryFileHeader) {
        const auto& central = centralDirectoryFileHeader;
        versionNeededToExtract = central.versionNeededToExtract;
        generalPurposeBitFlag = central.generalPurposeBitFlag;
        compressionMethod = central.compressionMethod;
        lastModificationTime = central.lastModificationTime;
        lastModificationDate = central.lastModificationDate;
        crc32 = central.crc32;
        compressedSize = central.compressedSize;
        unCompressedSize = central.unCompressedSize;
        
        fileName = centralDirectoryFileHeader.fileName;
        fileNameLength = static_cast<u16>(fileName.length());
    }
    
    void ZipLocalFileHeaderBase::deserializeBase(std::istream& stream) {
        // packing the struct causes issues with references
//        ::deserialize(stream, *this);
        
        ::deserialize(stream, signature);
        ::deserialize(stream, versionNeededToExtract);
        ::deserialize(stream, generalPurposeBitFlag);
        ::deserialize(stream, compressionMethod);
        ::deserialize(stream, compressionMethod);
        ::deserialize(stream, lastModificationTime);
        ::deserialize(stream, lastModificationDate);
        ::deserialize(stream, crc32);
        ::deserialize(stream, compressedSize);
        ::deserialize(stream, unCompressedSize);
        ::deserialize(stream, fileNameLength);
        ::deserialize(stream, extraFieldLength);
    }
    
    void ZipLocalFileHeaderBase::serializeBase(std::ostream& stream) {
        // packing the struct causes issues with references
//        ::serialize(stream, *this);
        
        ::serialize(stream, signature);
        ::serialize(stream, versionNeededToExtract);
        ::serialize(stream, generalPurposeBitFlag);
        ::serialize(stream, compressionMethod);
        ::serialize(stream, compressionMethod);
        ::serialize(stream, lastModificationTime);
        ::serialize(stream, lastModificationDate);
        ::serialize(stream, crc32);
        ::serialize(stream, compressedSize);
        ::serialize(stream, unCompressedSize);
        ::serialize(stream, fileNameLength);
        ::serialize(stream, extraFieldLength);
    }
    
    bool ZipLocalFileHeader::deserialize(std::istream& stream) {
        deserializeBase(stream);
        
        // If there is not any other entry.
        if (stream.fail() || signature != SIGNATURE_CONST) {
            stream.clear();
            const auto offset = static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount();
            stream.seekg(static_cast<std::ios::off_type>(offset), std::ios::beg);
            return false;
        }
        
        ::deserialize(stream, fileName, fileNameLength);
        
        if (extraFieldLength > 0) {
            ZipGenericExtraField extraField;
            
            auto extraFieldEnd = extraFieldLength + stream.tellg();
            
            while (extraField.deserialize(stream, extraFieldEnd)) {
                extraFields.push_back(extraField);
            }
            
            // Some archives do not store extra field in the form of tag, size and data tuples.
            // That may cause the above while cycle exit prior to reaching the extra field end,
            // which causes wrong data offset returned by ZipArchiveEntry::GetOffsetOfCompressedData().
            // Seek forcefully to the end of extra field to mitigate that problem.
            stream.seekg(extraFieldEnd, std::ios::beg);
        }
        
        return true;
    }
    
    void ZipLocalFileHeader::serialize(std::ostream& stream) {
        fileNameLength = static_cast<u16>(fileName.length());
        extraFieldLength = 0;
        
        for (auto& extraField : extraFields) {
            extraFieldLength += extraField.size();
        }
        
        serializeBase(stream);
        
        ::serialize(stream, fileName);
        
        if (extraFieldLength > 0) {
            for (auto& extraField : extraFields) {
                extraField.serialize(stream);
            }
        }
    }
    
    void ZipLocalFileHeader::deserializeAsDataDescriptor(std::istream& stream) {
        u32 firstWord;
        ::deserialize(stream, firstWord);
        
        // the signature is optional, if it's missing,
        // we're starting with crc32
        if (firstWord != DATA_DESCRIPTOR_SIGNATURE) {
            ::deserialize(stream, crc32);
        } else {
            crc32 = firstWord;
        }
        
        ::deserialize(stream, compressedSize);
        ::deserialize(stream, unCompressedSize);
    }
    
    void ZipLocalFileHeader::serializeAsDataDescriptor(std::ostream& stream) {
        ::serialize(stream, DATA_DESCRIPTOR_SIGNATURE);
        ::serialize(stream, crc32);
        ::serialize(stream, compressedSize);
        ::serialize(stream, unCompressedSize);
    }
    
}
