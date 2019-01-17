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
    
    bool ZipLocalFileHeader::deserialize(std::istream& stream) {
        ::deserialize<ZipLocalFileHeaderBase>(stream, *this);
        
        // If there is not any other entry.
        if (stream.fail() || signature != SIGNATURE_CONST) {
            stream.clear();
            const auto offset = static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount();
            stream.seekg(static_cast<std::ios::off_type>(offset),std::ios::beg);
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
    
        ::serialize<ZipLocalFileHeaderBase>(stream, *this);
        
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
            auto _crc32 = crc32;
            ::deserialize(stream, _crc32);
            crc32 = _crc32;
        } else {
            crc32 = firstWord;
        }
        
        // can't make pointer from packed fields
        auto _compressedSize = compressedSize;
        auto _unCompressedSize = unCompressedSize;
        ::deserialize(stream, _compressedSize);
        ::deserialize(stream, _unCompressedSize);
        compressedSize = _compressedSize;
        unCompressedSize = _unCompressedSize;
    }
    
    void ZipLocalFileHeader::serializeAsDataDescriptor(std::ostream& stream) {
        ::serialize(stream, DATA_DESCRIPTOR_SIGNATURE);
        ::serialize(stream, crc32);
        ::serialize(stream, compressedSize);
        ::serialize(stream, unCompressedSize);
    }
    
}
