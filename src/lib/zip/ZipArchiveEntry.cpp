#include <memory>

#include "ZipArchiveEntry.h"
#include "ZipArchive.h"

#include "methods/ZipMethodResolver.h"

#include "streams/zip_cryptostream.h"
#include "streams/compression_encoder_stream.h"
#include "streams/compression_decoder_stream.h"
#include "streams/nullstream.h"

#include "utils/stream_utils.h"
#include "utils/time_utils.h"

#include "src/main/util/strings.h"

#include <sstream>

namespace {
    
    using namespace std::string_literals;
    
    bool isValidFileName(std::string_view fullPath) {
        // this function ensures, that the filename will have non-zero
        // length, when the filename will be normalized
        if (fullPath.length() > 0) {
            std::string tmpFilename(fullPath);
            std::replace(tmpFilename.begin(), tmpFilename.end(), '\\', '/');
            
            // if the filename is built only from '/', then it is invalid path
            return tmpFilename.find_first_not_of('/') != std::string::npos;
        }
        return false;
    }
    
    void checkFileName(std::string_view fullPath) {
        if (!isValidFileName(fullPath)) {
            throw std::runtime_error("bad file name: "s + fullPath);
        }
    }
    
    std::string_view getFileNameFromPath(std::string_view fullPath) {
        const auto dirSeparatorPos = fullPath.find_last_of('/');
        if (dirSeparatorPos != std::string::npos) {
            return fullPath.substr(dirSeparatorPos + 1);
        } else {
            return fullPath;
        }
    }
    
    bool isDirectoryPath(std::string_view fullPath) {
        return fullPath.length() > 0 && fullPath.back() == '/';
    }
    
}

ZipArchiveEntry::~ZipArchiveEntry() {
    closeRawStream();
    closeDecompressionStream();
}

//////////////////////////////////////////////////////////////////////////
// public methods & getters & setters

std::string_view ZipArchiveEntry::fullName() const noexcept {
    return fileHeader.central.fileName;
}

void ZipArchiveEntry::setFullName(std::string_view fullName) {
    std::string fileName(fullName);
    
    // unify slashes
    std::replace(fileName.begin(), fileName.end(), '\\', '/');
    
    bool isDirectory = isDirectoryPath(fileName);
    
    // if slash is first char, remove it
    if (fileName[0] == '/') {
        fileName.erase(0, fileName.find_first_not_of('/'));
    }
    
    // find multiply slashes
    std::string correctFileName;
    correctFileName.reserve(fileName.size());
    bool prevWasSlash = false;
    for (auto c : fileName) {
        if (c == '/' && prevWasSlash) {
            continue;
        }
        prevWasSlash = (c == '/');
        correctFileName += c;
    }
    correctFileName.shrink_to_fit();
    fileHeader.central.fileName = correctFileName;
    _name = getFileNameFromPath(correctFileName);
    
    setAttributes(isDirectory ? Attributes::Directory : Attributes::Archive);
}

std::string_view ZipArchiveEntry::name() const noexcept {
    return _name;
}

void ZipArchiveEntry::setName(std::string_view name) {
    // search for '/' in path name.
    // if this entry is directory, search up to one character
    // before the last one (which is '/')
    // if this entry is file, just search until last '/'
    // will be found
    const std::string_view fullName = this->fullName();
    const auto index = (u32) attributes() & (u32) Attributes::Archive
                       ? std::string::npos
                       : fullName.length() - 1;
    const auto dirDelimiterPos = fullName.find_last_of('/', index);
    
    std::string newName;
    if (dirDelimiterPos != std::string::npos) {
        newName += fullName.substr(0, dirDelimiterPos); // folder
    }
    newName += name;
    if (isDirectory()) {
        newName += '/';
    }
    
    setFullName(newName);
}

std::string_view ZipArchiveEntry::comment() const noexcept {
    return fileHeader.central.fileComment;
}

void ZipArchiveEntry::setComment(std::string_view comment) {
    fileHeader.central.fileComment = comment;
}

time_t ZipArchiveEntry::lastWriteTime() const noexcept {
    const auto& central = fileHeader.central;
    return utils::time::dateTimeToTimeStamp(central.lastModificationDate, central.lastModificationTime);
}

void ZipArchiveEntry::setLastWriteTime(time_t modTime) noexcept {
    auto& central = fileHeader.central;
    utils::time::timeStampToDateTime(modTime, central.lastModificationDate, central.lastModificationTime);
}

ZipArchiveEntry::Attributes ZipArchiveEntry::attributes() const noexcept {
    return static_cast<Attributes>(fileHeader.central.externalFileAttributes);
}

void ZipArchiveEntry::setAttributes(Attributes value) {
    const Attributes prevVal = attributes();
    Attributes newVal = prevVal | value;
    auto& central = fileHeader.central;
    auto& fileName = central.fileName;
    
    if (!!(prevVal & Attributes::Directory) && !!(newVal & Attributes::Archive)) {
        // if we're changing from directory to file
        newVal &= ~Attributes::Directory;
        
        if (isDirectoryPath(fileName)) {
            fileName.pop_back();
        }
    } else if (!!(prevVal & Attributes::Archive) && !!(newVal & Attributes::Directory)) {
        // if we're changing from file to directory
        newVal &= ~Attributes::Archive;
        
        if (!isDirectoryPath(fileName)) {
            fileName += '/';
        }
    }
    
    // if this entry is directory, ensure that crc32 & sizes
    // are set to 0 and do not include any stream
    if (!!(newVal & Attributes::Directory)) {
        central.crc32 = 0;
        central.compressedSize = 0;
        central.unCompressedSize = 0;
    }
    
    central.externalFileAttributes = static_cast<u32>(newVal);
}

const u16& ZipArchiveEntry::compressionMethod() const noexcept {
    return fileHeader.central.compressionMethod;
}

u16& ZipArchiveEntry::compressionMethod() noexcept {
    return fileHeader.central.compressionMethod;
}

bool ZipArchiveEntry::isPasswordProtected() const noexcept {
    return !!(generalPurposeBitFlag() & BitFlag::Encrypted);
}

std::string_view ZipArchiveEntry::password() const noexcept {
    return _password;
}

void ZipArchiveEntry::setPassword(std::string_view password) {
    _password = password;
    
    // allow unset password only for empty files
    if (!originallyInArchive || (hasLocalFileHeader && size() == 0)) {
        setGeneralPurposeBitFlag(BitFlag::Encrypted, !_password.empty());
    }
}

u32 ZipArchiveEntry::crc32() const noexcept {
    return fileHeader.central.crc32;
}

size_t ZipArchiveEntry::size() const noexcept {
    return fileHeader.central.unCompressedSize;
}


size_t ZipArchiveEntry::compressedSize() const noexcept {
    return fileHeader.central.compressedSize;
}

bool ZipArchiveEntry::canExtract() const noexcept {
    return (versionToExtract() <= VERSION_MADE_BY_DEFAULT);
}

bool ZipArchiveEntry::isDirectory() const noexcept {
    return !!(attributes() & Attributes::Directory);
}

bool ZipArchiveEntry::isUsingDataDescriptor() const noexcept {
    return !!(generalPurposeBitFlag() & BitFlag::DataDescriptor);
}

void ZipArchiveEntry::useDataDescriptor(bool use) noexcept {
    setGeneralPurposeBitFlag(BitFlag::DataDescriptor, use);
}

std::istream* ZipArchiveEntry::rawStream() {
    if (_rawStream == nullptr) {
        if (originallyInArchive) {
            const auto offsetOfCompressedData = seekToCompressedData();
            _rawStream = std::make_shared<isubstream>(
                    *archive.stream, offsetOfCompressedData, compressedSize());
        } else {
            _rawStream = std::make_shared<isubstream>(*immediateBuffer);
        }
    }
    return _rawStream.get();
}

std::istream* ZipArchiveEntry::decompressionStream() {
    std::shared_ptr<std::istream> intermediateStream;
    
    // there shouldn't be opened another stream
    if (canExtract() && archiveStream == nullptr && encryptionStream == nullptr) {
        const auto offsetOfCompressedData = seekToCompressedData();
        const bool needsPassword = !!(generalPurposeBitFlag() & BitFlag::Encrypted);
        const bool needsDecompress = compressionMethod() != StoreMethod::CompressionMethod;
        
        if (needsPassword && _password.empty()) {
            // we need password, but we does not have it
            return nullptr;
        }
        
        // make correctly-ended substream of the input stream
        intermediateStream = archiveStream = std::make_shared<isubstream>(
                *archive.stream, offsetOfCompressedData, compressedSize());
        
        if (needsPassword) {
            const std::shared_ptr<zip_cryptostream> cryptoStream = std::make_shared<zip_cryptostream>(
                    *intermediateStream,
                    _password.c_str());
            cryptoStream->set_final_byte(lastByteOfEncryptionHeader());
            const bool hasCorrectPassword = cryptoStream->prepare_for_decryption();
            
            // set it here, because in case the hasCorrectPassword is false
            // the method CloseDecompressionStream() will properly delete the stream
            intermediateStream = encryptionStream = cryptoStream;
            
            if (!hasCorrectPassword) {
                closeDecompressionStream();
                return nullptr;
            }
        }
        
        if (needsDecompress) {
            ICompressionMethod::Ptr zipMethod = ZipMethodResolver::GetZipMethodInstance(compressionMethod());
            
            if (zipMethod != nullptr) {
                intermediateStream = compressionStream = std::make_shared<compression_decoder_stream>(
                        zipMethod->GetDecoder(), zipMethod->GetDecoderProperties(), *intermediateStream);
            }
        }
    }
    
    return intermediateStream.get();
}

bool ZipArchiveEntry::isRawStreamOpened() const noexcept {
    return _rawStream != nullptr;
}

bool ZipArchiveEntry::isDecompressionStreamOpened() const noexcept {
    return compressionStream != nullptr;
}

void ZipArchiveEntry::closeRawStream() {
    _rawStream.reset();
}

void ZipArchiveEntry::closeDecompressionStream() {
    compressionStream.reset();
    encryptionStream.reset();
    archiveStream.reset();
    immediateBuffer.reset();
}

bool ZipArchiveEntry::setCompressionStream(std::istream& stream,
                                           ICompressionMethod::Ptr method /* = DeflateMethod::Create() */,
                                           CompressionMode mode /* = CompressionMode::Deferred */) {
    // if _inputStream is set, we already have some stream to compress
    // so we discard it
    if (inputStream != nullptr) {
        unloadCompressionData();
    }
    
    isNewOrChanged = true;
    
    inputStream = &stream;
    compressionMethod() = method->GetZipMethodDescriptor().GetCompressionMethod();
    _compressionMethod = std::move(method);
    _compressionMode = mode;
    
    if (inputStream != nullptr && _compressionMode == CompressionMode::Immediate) {
        immediateBuffer = std::make_shared<std::stringstream>();
        internalCompressStream(*inputStream, *immediateBuffer);
        
        // we have everything we need, let's act like we were loaded from archive :)
        isNewOrChanged = false;
        inputStream = nullptr;
    }
    
    return true;
}

void ZipArchiveEntry::unSetCompressionStream() {
    if (!hasCompressionStream()) {
        fetchLocalFileHeader();
    }
    unloadCompressionData();
    setPassword(std::string());
}

void ZipArchiveEntry::remove() {
    size_t i = 0;
    for (const auto& entry : archive) {
        if (&entry == this) {
            archive.removeEntry(i);
            return;
        }
        i++;
    }
    assert(false);
}

// private getters & setters

ZipArchiveEntry::BitFlag ZipArchiveEntry::generalPurposeBitFlag() const noexcept {
    return static_cast<BitFlag>(fileHeader.central.generalPurposeBitFlag);
}

BitFlagSetter<ZipArchiveEntry::BitFlag> ZipArchiveEntry::generalPurposeBitFlagRef() noexcept {
    return BitFlagSetter<BitFlag>(fileHeader.central.generalPurposeBitFlag);
}

void ZipArchiveEntry::setGeneralPurposeBitFlag(BitFlag flag, bool set) noexcept {
    auto flags = generalPurposeBitFlagRef();
    if (set) {
        flags |= flag;
    } else {
        flags &= ~flag;
    }
}

const u16& ZipArchiveEntry::versionToExtract() const noexcept {
    return fileHeader.central.versionNeededToExtract;
}

u16& ZipArchiveEntry::versionToExtract() noexcept {
    return fileHeader.central.versionNeededToExtract;
}

const u16& ZipArchiveEntry::versionMadeBy() const noexcept {
    return fileHeader.central.versionMadeBy;
}

u16& ZipArchiveEntry::versionMadeBy() noexcept {
    return fileHeader.central.versionMadeBy;
}

const i32& ZipArchiveEntry::offsetOfLocalHeader() const noexcept {
    return fileHeader.central.relativeOffsetOfLocalHeader;
}

i32& ZipArchiveEntry::offsetOfLocalHeader() noexcept {
    return fileHeader.central.relativeOffsetOfLocalHeader;
}

bool ZipArchiveEntry::hasCompressionStream() const noexcept {
    return inputStream != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// private working methods

void ZipArchiveEntry::fetchLocalFileHeader() {
    auto& stream = *archive.stream;
    if (!hasLocalFileHeader && originallyInArchive) {
        stream.seekg(offsetOfLocalHeader(), std::ios::beg);
        fileHeader.local.deserialize(stream);
        offset.compressedData = stream.tellg();
    }
    
    // sync data
    syncLocalWithCentralDirectoryFileHeader();
    hasLocalFileHeader = true;
}

void ZipArchiveEntry::checkFileNameCorrection() {
    // this forces recheck of the filename.
    // this is useful when the check is needed after
    // deserialization
    setFullName(fullName());
}

void ZipArchiveEntry::fixVersionToExtractAtLeast(u16 value) {
    if (versionToExtract() < value) {
        versionToExtract() = value;
    }
}

void ZipArchiveEntry::syncLocalWithCentralDirectoryFileHeader() {
    fileHeader.local.syncWithCentralDirectoryFileHeader(fileHeader.central);
}

void ZipArchiveEntry::syncCentralDirectoryWithLocalFileHeader() {
    fileHeader.central.syncWithLocalFileHeader(fileHeader.local);
    fixVersionToExtractAtLeast(isDirectory()
                               ? VERSION_NEEDED_EXPLICIT_DIRECTORY
                               : _compressionMethod->GetZipMethodDescriptor().GetVersionNeededToExtract());
}

std::ios::pos_type ZipArchiveEntry::offsetOfCompressedData() {
    if (!hasLocalFileHeader) {
        fetchLocalFileHeader();
    }
    return offset.compressedData;
}

std::ios::pos_type ZipArchiveEntry::seekToCompressedData() {
    // check for fail bit?
    archive.stream->seekg(offsetOfCompressedData(), std::ios::beg);
    return offsetOfCompressedData();
}

void ZipArchiveEntry::serializeLocalFileHeader(std::ostream& stream) {
    // ensure opening the stream
    std::istream* compressedDataStream = nullptr;
    
    if (!isDirectory()) {
        if (inputStream == nullptr) {
            if (!isNewOrChanged) {
                // the file was either compressed in immediate mode,
                // or was in previous archive
                compressedDataStream = rawStream();
            }
            
            // if file is new and empty or stream has been set to nullptr,
            // just do not set any compressed data stream
        } else {
            assert(isNewOrChanged);
            compressedDataStream = inputStream;
        }
    }
    
    if (!hasLocalFileHeader) {
        fetchLocalFileHeader();
    }
    
    // save offset of stream here
    offset.serializedLocalFileHeader = stream.tellp();
    
    auto& local = fileHeader.local;
    
    if (isUsingDataDescriptor()) {
        local.compressedSize = 0;
        local.unCompressedSize = 0;
        local.crc32 = 0;
    }
    
    local.serialize(stream);
    
    // if this entry is a directory, it should not contain any data
    // nor crc.
    if (isDirectory()) {
        assert(!crc32() && !size() && !compressedSize() && !inputStream);
    }
    
    if (!isDirectory() && compressedDataStream != nullptr) {
        if (isNewOrChanged) {
            internalCompressStream(*compressedDataStream, stream);
            
            if (isUsingDataDescriptor()) {
                local.serializeAsDataDescriptor(stream);
            } else {
                // actualize local file header
                // make non-seekable version?
                stream.seekp(offset.serializedLocalFileHeader);
                local.serialize(stream);
                stream.seekp(compressedSize(), std::ios::cur);
            }
        } else {
            utils::stream::copy(*compressedDataStream, stream);
        }
    }
}

void ZipArchiveEntry::serializeCentralDirectoryFileHeader(std::ostream& stream) {
    auto& central = fileHeader.central;
    central.relativeOffsetOfLocalHeader = static_cast<i32>(offset.serializedLocalFileHeader);
    central.serialize(stream);
}

void ZipArchiveEntry::unloadCompressionData() {
    // unload stream
    immediateBuffer->clear();
    inputStream = nullptr;
    
    auto& central = fileHeader.central;
    central.compressedSize = 0;
    central.unCompressedSize = 0;
    central.crc32 = 0;
}

void ZipArchiveEntry::internalCompressStream(std::istream& inputStream, std::ostream& outputStream) {
    std::ostream* intermediateStream = &outputStream;
    
    std::unique_ptr<zip_cryptostream> cryptoStream;
    if (!_password.empty()) {
        generalPurposeBitFlagRef() |= BitFlag::Encrypted;
        
        cryptoStream = std::make_unique<zip_cryptostream>();
        
        cryptoStream->init(outputStream, _password.c_str());
        cryptoStream->set_final_byte(lastByteOfEncryptionHeader());
        intermediateStream = cryptoStream.get();
    }
    
    crc32stream crc32Stream;
    crc32Stream.init(inputStream);
    
    compression_encoder_stream compressionStream(
            _compressionMethod->GetEncoder(),
            _compressionMethod->GetEncoderProperties(),
            *intermediateStream);
    intermediateStream = &compressionStream;
    utils::stream::copy(crc32Stream, *intermediateStream);
    
    intermediateStream->flush();
    
    auto& local = fileHeader.local;
    local.unCompressedSize = static_cast<u32>(compressionStream.get_bytes_read());
    local.compressedSize = static_cast<u32>(compressionStream.get_bytes_written() +
                                            (!_password.empty() ? 12 : 0));
    local.crc32 = crc32Stream.get_crc32();
    
    syncCentralDirectoryWithLocalFileHeader();
}

void ZipArchiveEntry::figureCrc32() {
    if (isDirectory() || inputStream == nullptr || !isNewOrChanged) {
        return;
    }
    
    // stream must be seekable
    auto position = inputStream->tellg();
    
    // compute crc32
    crc32stream crc32Stream;
    crc32Stream.init(*inputStream);
    
    // just force to read all from crc32stream
    nullstream devNull;
    utils::stream::copy(crc32Stream, devNull);
    
    // seek back
    inputStream->clear();
    inputStream->seekg(position);
    
    fileHeader.central.crc32 = crc32Stream.get_crc32();
}

u32 ZipArchiveEntry::lastUintOfEncryptionHeader() {
    if (!!(generalPurposeBitFlag() & BitFlag::DataDescriptor)) {
        // In the case that bit 3 of the general purpose bit flag is set to
        // indicate the presence of a 'data descriptor' (signature
        // 0x08074b50), the last byte of the decrypted header is sometimes
        // compared with the high-order byte of the lastmodified time,
        // rather than the high-order byte of the CRC, to verify the
        // password.
        //
        // This is not documented in the PKWare Appnote.txt.
        // This was discovered this by analysis of the Crypt.c source file in the
        // InfoZip library
        // http://www.info-zip.org/pub/infozip/
        
        // Also, winzip insists on this!
        return fileHeader.central.lastModificationTime >> 8u;
    } else {
        // When bit 3 is not set, the CRC value is required before
        // encryption of the file data begins. In this case there is no way
        // around it: must read the stream in its entirety to compute the
        // actual CRC before proceeding.
        figureCrc32();
        return crc32() >> 24u;
    }
}

u8 ZipArchiveEntry::lastByteOfEncryptionHeader() {
    return static_cast<u8>(lastUintOfEncryptionHeader() & 0xFF);
}

ZipArchiveEntry::ZipArchiveEntry(ConstructorKey key [[maybe_unused]],
                                 ZipArchive& archive, size_t index, std::string_view fullPath)
        : archive(archive),
          index(index),
          isNewOrChanged(true) {
    checkFileName(fullPath);
    setAttributes(Attributes::Archive);
    versionToExtract() = VERSION_NEEDED_DEFAULT;
    versionMadeBy() = VERSION_MADE_BY_DEFAULT;
    setLastWriteTime(time(nullptr));
    setFullName(fullPath);
    compressionMethod() = StoreMethod::CompressionMethod;
    generalPurposeBitFlagRef() |= BitFlag::None;
}

ZipArchiveEntry::ZipArchiveEntry(ConstructorKey key [[maybe_unused]],
                                 ZipArchive& archive, size_t index, const Central& central)
        : archive(archive),
          index(index),
          originallyInArchive(true),
          fileHeader({}) {
    checkFileName(central.fileName);
    fileHeader.central = central;
    checkFileNameCorrection();
    
    // determining folder by path has more priority
    // than attributes. however, if attributes
    // does not correspond with path, they will be fixed.
    setAttributes(isDirectoryPath(fullName()) ? Attributes::Directory : Attributes::Archive);
}
