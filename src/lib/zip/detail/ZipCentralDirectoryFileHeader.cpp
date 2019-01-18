#include "ZipCentralDirectoryFileHeader.h"
#include "ZipLocalFileHeader.h"

#include <cstring>
#include <ctime>

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    ZipCentralDirectoryFileHeader::ZipCentralDirectoryFileHeader() : ZipCentralDirectoryFileHeaderBase({}) {
        signature = SIGNATURE_CONSTANT;
    }
    
    void ZipCentralDirectoryFileHeader::syncWithLocalFileHeader(const ZipLocalFileHeader& localFileHeader) {
        const auto& local = localFileHeader;
        crc32 = local.crc32;
        compressedSize = local.compressedSize;
        unCompressedSize = local.unCompressedSize;
        
        fileNameLength = static_cast<u16>(fileName.length());
        fileCommentLength = static_cast<u16>(fileComment.length());
    }
    
    void ZipCentralDirectoryFileHeaderBase::deserializeBase(std::istream& stream) {
        // packing the struct causes issues with references
//        ::deserialize(stream, *this);
        
        ::deserialize(stream, signature);
        ::deserialize(stream, versionMadeBy);
        ::deserialize(stream, versionNeededToExtract);
        ::deserialize(stream, generalPurposeBitFlag);
        ::deserialize(stream, compressionMethod);
        ::deserialize(stream, lastModificationTime);
        ::deserialize(stream, lastModificationDate);
        ::deserialize(stream, crc32);
        ::deserialize(stream, compressedSize);
        ::deserialize(stream, unCompressedSize);
        ::deserialize(stream, fileNameLength);
        ::deserialize(stream, extraFieldLength);
        ::deserialize(stream, fileCommentLength);
        ::deserialize(stream, diskNumberStart);
        ::deserialize(stream, internalFileAttributes);
        ::deserialize(stream, externalFileAttributes);
        ::deserialize(stream, relativeOffsetOfLocalHeader);
    }
    
    void ZipCentralDirectoryFileHeaderBase::serializeBase(std::ostream& stream) {
        // packing the struct causes issues with references
//        ::serialize(stream, *this);
    
        ::serialize(stream, signature);
        ::serialize(stream, versionMadeBy);
        ::serialize(stream, versionNeededToExtract);
        ::serialize(stream, generalPurposeBitFlag);
        ::serialize(stream, compressionMethod);
        ::serialize(stream, lastModificationTime);
        ::serialize(stream, lastModificationDate);
        ::serialize(stream, crc32);
        ::serialize(stream, compressedSize);
        ::serialize(stream, unCompressedSize);
        ::serialize(stream, fileNameLength);
        ::serialize(stream, extraFieldLength);
        ::serialize(stream, fileCommentLength);
        ::serialize(stream, diskNumberStart);
        ::serialize(stream, internalFileAttributes);
        ::serialize(stream, externalFileAttributes);
        ::serialize(stream, relativeOffsetOfLocalHeader);
    }
    
    bool ZipCentralDirectoryFileHeader::deserialize(std::istream& stream) {
        deserializeBase(stream);
        
        // If there is not any other entry.
        if (stream.fail() || signature != SIGNATURE_CONSTANT) {
            stream.clear();
            auto offset = static_cast<std::ios::streamoff>(stream.tellg()) - stream.gcount();
            stream.seekg(static_cast<std::ios::off_type>(offset), std::istream::beg);
            return false;
        }
        
        ::deserialize(stream, fileName, fileNameLength);
        
        if (extraFieldLength > 0) {
            ZipGenericExtraField extraField;
            auto extraFieldEnd = extraFieldLength + stream.tellg();
            while (extraField.deserialize(stream, extraFieldEnd)) {
                extraFields.push_back(extraField);
            }
        }
        
        ::deserialize(stream, fileComment, fileCommentLength);
        
        return true;
    }
    
    void ZipCentralDirectoryFileHeader::serialize(std::ostream& stream) {
        fileNameLength = static_cast<uint16_t>(fileName.length());
        fileCommentLength = static_cast<uint16_t>(fileComment.length());
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
        
        ::serialize(stream, fileComment);
    }
    
}
