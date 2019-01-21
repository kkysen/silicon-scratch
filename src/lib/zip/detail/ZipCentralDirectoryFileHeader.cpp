#include "ZipCentralDirectoryFileHeader.h"
#include "ZipLocalFileHeader.h"

#include <cstring>
#include <ctime>

#include "src/lib/zip/streams/serialization.h"

namespace detail {
    
    ZipCentralDirectoryFileHeader::ZipCentralDirectoryFileHeader()
    : ZipCentralDirectoryFileHeaderBase1({}), ZipCentralDirectoryFileHeaderBase2({}) {
        signature = constants::signature;
    }
    
    void ZipCentralDirectoryFileHeader::syncWithLocalFileHeader(const ZipLocalFileHeader& localFileHeader) {
        const auto& local = localFileHeader;
        crc32 = local.crc32;
        compressedSize = local.compressedSize;
        unCompressedSize = local.unCompressedSize;
        
        fileNameLength = static_cast<u16>(fileName.length());
        fileCommentLength = static_cast<u16>(fileComment.length());
    }
    
    void ZipCentralDirectoryFileHeaderBase1::deserializeBase1(std::istream& stream) {
        ::deserialize<sizeof(padding1)>(stream, *this);
    }
    
    void ZipCentralDirectoryFileHeaderBase1::serializeBase1(std::ostream& stream) const {
        ::serialize<sizeof(padding1)>(stream, *this);
    }
    
    void ZipCentralDirectoryFileHeaderBase2::deserializeBase2(std::istream& stream) {
        ::deserialize<sizeof(padding2)>(stream, *this);
    }
    
    void ZipCentralDirectoryFileHeaderBase2::serializeBase2(std::ostream& stream) const {
        ::serialize<sizeof(padding2)>(stream, *this);
    }
    
    bool ZipCentralDirectoryFileHeader::deserialize(std::istream& stream) {
        deserializeBase1(stream);
        deserializeBase2(stream);
        
        // If there is not any other entry.
        if (stream.fail() || signature != constants::signature) {
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
    
    void ZipCentralDirectoryFileHeader::serialize(std::ostream& stream) const {
        fileNameLength = static_cast<uint16_t>(fileName.length());
        fileCommentLength = static_cast<uint16_t>(fileComment.length());
        extraFieldLength = 0;
        
        for (auto& extraField : extraFields) {
            extraFieldLength += extraField.size();
        }
    
        serializeBase1(stream);
        serializeBase2(stream);
        
        ::serialize(stream, fileName);
        
        if (extraFieldLength > 0) {
            for (auto& extraField : extraFields) {
                extraField.serialize(stream);
            }
        }
        
        ::serialize(stream, fileComment);
    }
    
}
