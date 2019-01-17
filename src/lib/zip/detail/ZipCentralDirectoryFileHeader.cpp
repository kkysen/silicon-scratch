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
    
    bool ZipCentralDirectoryFileHeader::deserialize(std::istream& stream) {
        ::deserialize<ZipCentralDirectoryFileHeaderBase>(stream, *this);
        
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
    
        ::serialize<ZipCentralDirectoryFileHeaderBase>(stream, *this);
        
        ::serialize(stream, fileName);
        
        if (extraFieldLength > 0) {
            for (auto& extraField : extraFields) {
                extraField.serialize(stream);
            }
        }
        
        ::serialize(stream, fileComment);
    }
    
}
