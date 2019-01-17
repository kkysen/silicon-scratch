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

#include <sstream>

namespace {
    
    bool isValidFilename(const std::string& fullPath) {
        // this function ensures, that the filename will have non-zero
        // length, when the filename will be normalized
        
        if (fullPath.length() > 0) {
            std::string tmpFilename = fullPath;
            std::replace(tmpFilename.begin(), tmpFilename.end(), '\\', '/');
            
            // if the filename is built only from '/', then it is invalid path
            return tmpFilename.find_first_not_of('/') != std::string::npos;
        }
        
        return false;
    }
    
    std::string getFilenameFromPath(const std::string& fullPath) {
        std::string::size_type dirSeparatorPos;
        
        if ((dirSeparatorPos = fullPath.find_last_of('/')) != std::string::npos) {
            return fullPath.substr(dirSeparatorPos + 1);
        } else {
            return fullPath;
        }
    }
    
    bool isDirectoryPath(const std::string& fullPath) {
        return (fullPath.length() > 0 && fullPath.back() == '/');
    }
    
}

ZipArchiveEntry::ZipArchiveEntry() = default;

ZipArchiveEntry::~ZipArchiveEntry() {
    this->closeRawStream();
    this->closeDecompressionStream();
}

ZipArchiveEntry::Ptr ZipArchiveEntry::CreateNew(ZipArchive* zipArchive, const std::string& fullPath) {
    ZipArchiveEntry::Ptr result;
    
    assert(zipArchive != nullptr);
    
    if (isValidFilename(fullPath)) {
        result.reset(new ZipArchiveEntry());
        
        result->_archive = zipArchive;
        result->_isNewOrChanged = true;
        result->setAttributes(Attributes::Archive);
        result->setVersionToExtract(VERSION_NEEDED_DEFAULT);
        result->setVersionMadeBy(VERSION_MADE_BY_DEFAULT);
        result->setLastWriteTime(time(nullptr));
        
        result->setFullName(fullPath);
        
        result->setCompressionMethod(StoreMethod::CompressionMethod);
        result->setGeneralPurposeBitFlag(BitFlag::None);
    }
    
    return result;
}

ZipArchiveEntry::Ptr
ZipArchiveEntry::CreateExisting(ZipArchive* zipArchive, detail::ZipCentralDirectoryFileHeader& cd) {
    ZipArchiveEntry::Ptr result;
    
    assert(zipArchive != nullptr);
    
    if (isValidFilename(cd.fileName)) {
        result.reset(new ZipArchiveEntry());
        
        result->_archive = zipArchive;
        result->_centralDirectoryFileHeader = cd;
        result->_originallyInArchive = true;
        result->checkFilenameCorrection();
        
        // determining folder by path has more priority
        // than attributes. however, if attributes
        // does not correspond with path, they will be fixed.
        result->setAttributes(isDirectoryPath(result->fullName())
                              ? Attributes::Directory
                              : Attributes::Archive);
    }
    
    return result;
}

//////////////////////////////////////////////////////////////////////////
// public methods & getters & setters

const std::string& ZipArchiveEntry::fullName() const {
    return _centralDirectoryFileHeader.fileName;
}

void ZipArchiveEntry::setFullName(const std::string& fullName) {
    std::string filename = fullName;
    std::string correctFilename;
    
    // unify slashes
    std::replace(filename.begin(), filename.end(), '\\', '/');
    
    bool isDirectory = isDirectoryPath(filename);
    
    // if slash is first char, remove it
    if (filename[0] == '/') {
        filename = filename.substr(filename.find_first_not_of('/'));
    }
    
    // find multiply slashes
    bool prevWasSlash = false;
    for (std::string::size_type i = 0; i < filename.length(); ++i) {
        if (filename[i] == '/' && prevWasSlash) continue;
        prevWasSlash = (filename[i] == '/');
        
        correctFilename += filename[i];
    }
    
    _centralDirectoryFileHeader.fileName = correctFilename;
    _name = getFilenameFromPath(correctFilename);
    
    this->setAttributes(isDirectory ? Attributes::Directory : Attributes::Archive);
}

const std::string& ZipArchiveEntry::name() const {
    return _name;
}

void ZipArchiveEntry::setName(const std::string& name) {
    std::string folder;
    std::string::size_type dirDelimiterPos;
    
    // search for '/' in path name.
    // if this entry is directory, search up to one character
    // before the last one (which is '/')
    // if this entry is file, just search until last '/'
    // will be found
    dirDelimiterPos = this->fullName().find_last_of('/',
                                                    (uint32_t) this->attributes() & (uint32_t) Attributes::Archive
                                                    ? std::string::npos
                                                    : this->fullName().length() - 1);
    
    if (dirDelimiterPos != std::string::npos) {
        folder = this->fullName().substr(0, dirDelimiterPos);
    }
    
    this->setFullName(folder + name);
    
    if (this->isDirectory()) {
        this->setFullName(this->fullName() + '/');
    }
}

const std::string& ZipArchiveEntry::comment() const {
    return _centralDirectoryFileHeader.fileComment;
}

void ZipArchiveEntry::setComment(const std::string& comment) {
    _centralDirectoryFileHeader.fileComment = comment;
}

time_t ZipArchiveEntry::lastWriteTime() const {
    return utils::time::dateTimeToTimeStamp(_centralDirectoryFileHeader.lastModificationDate,
                                            _centralDirectoryFileHeader.lastModificationTime);
}

void ZipArchiveEntry::setLastWriteTime(time_t modTime) {
    // ZipCentralDirectoryFileHeader is packed, so can't get a pointer to some fields
    auto lastModificationDate = _centralDirectoryFileHeader.lastModificationDate;
    auto lastModificationTime = _centralDirectoryFileHeader.lastModificationTime;
    utils::time::timeStampToDateTime(modTime, lastModificationDate, lastModificationTime);
    _centralDirectoryFileHeader.lastModificationDate = lastModificationDate;
    _centralDirectoryFileHeader.lastModificationTime = lastModificationTime;
}

ZipArchiveEntry::Attributes ZipArchiveEntry::attributes() const {
    return static_cast<Attributes>(_centralDirectoryFileHeader.externalFileAttributes);
}

uint16_t ZipArchiveEntry::compressionMethod() const {
    return _centralDirectoryFileHeader.compressionMethod;
}

void ZipArchiveEntry::setAttributes(Attributes value) {
    Attributes prevVal = this->attributes();
    Attributes newVal = prevVal | value;
    
    // if we're changing from directory to file
    if (!!(prevVal & Attributes::Directory) && !!(newVal & Attributes::Archive)) {
        newVal &= ~Attributes::Directory;
        
        if (isDirectoryPath(_centralDirectoryFileHeader.fileName)) {
            _centralDirectoryFileHeader.fileName.pop_back();
        }
    }
        
        // if we're changing from file to directory
    else if (!!(prevVal & Attributes::Archive) && !!(newVal & Attributes::Directory)) {
        newVal &= ~Attributes::Archive;
        
        if (!isDirectoryPath(_centralDirectoryFileHeader.fileName)) {
            _centralDirectoryFileHeader.fileName += '/';
        }
    }
    
    // if this entry is directory, ensure that crc32 & sizes
    // are set to 0 and do not include any stream
    if (!!(newVal & Attributes::Directory)) {
        _centralDirectoryFileHeader.crc32 = 0;
        _centralDirectoryFileHeader.compressedSize = 0;
        _centralDirectoryFileHeader.unCompressedSize = 0;
    }
    
    _centralDirectoryFileHeader.externalFileAttributes = static_cast<uint32_t>(newVal);
}

bool ZipArchiveEntry::isPasswordProtected() const {
    return !!(this->generalPurposeBitFlag() & BitFlag::Encrypted);
}

const std::string& ZipArchiveEntry::password() const {
    return _password;
}

void ZipArchiveEntry::setPassword(const std::string& password) {
    _password = password;
    
    // allow unset password only for empty files
    if (!_originallyInArchive || (_hasLocalFileHeader && this->size() == 0)) {
        this->setGeneralPurposeBitFlag(BitFlag::Encrypted, !_password.empty());
    }
}

uint32_t ZipArchiveEntry::crc32() const {
    return _centralDirectoryFileHeader.crc32;
}

size_t ZipArchiveEntry::size() const {
    return static_cast<size_t>(_centralDirectoryFileHeader.unCompressedSize);
}

size_t ZipArchiveEntry::compressedSize() const {
    return static_cast<size_t>(_centralDirectoryFileHeader.compressedSize);
}


bool ZipArchiveEntry::canExtract() const {
    return (this->versionToExtract() <= VERSION_MADE_BY_DEFAULT);
}

bool ZipArchiveEntry::isDirectory() const {
    return !!(this->attributes() & Attributes::Directory);
}

bool ZipArchiveEntry::isUsingDataDescriptor() const {
    return !!(this->generalPurposeBitFlag() & BitFlag::DataDescriptor);
}

void ZipArchiveEntry::useDataDescriptor(bool use) {
    this->setGeneralPurposeBitFlag(BitFlag::DataDescriptor, use);
}

std::istream* ZipArchiveEntry::rawStream() {
    if (_rawStream == nullptr) {
        if (_originallyInArchive) {
            auto offsetOfCompressedData = this->seekToCompressedData();
            _rawStream = std::make_shared<isubstream>(*_archive->zipStream, offsetOfCompressedData,
                                                      this->compressedSize());
        } else {
            _rawStream = std::make_shared<isubstream>(*_immediateBuffer);
        }
    }
    
    return _rawStream.get();
}

std::istream* ZipArchiveEntry::decompressionStream() {
    std::shared_ptr<std::istream> intermediateStream;
    
    // there shouldn't be opened another stream
    if (this->canExtract() && _archiveStream == nullptr && _encryptionStream == nullptr) {
        auto offsetOfCompressedData = this->seekToCompressedData();
        bool needsPassword = !!(this->generalPurposeBitFlag() & BitFlag::Encrypted);
        bool needsDecompress = this->compressionMethod() != StoreMethod::CompressionMethod;
        
        if (needsPassword && _password.empty()) {
            // we need password, but we does not have it
            return nullptr;
        }
        
        // make correctly-ended substream of the input stream
        intermediateStream = _archiveStream = std::make_shared<isubstream>(*_archive->zipStream,
                                                                           offsetOfCompressedData,
                                                                           this->compressedSize());
        
        if (needsPassword) {
            std::shared_ptr<zip_cryptostream> cryptoStream = std::make_shared<zip_cryptostream>(*intermediateStream,
                                                                                                _password.c_str());
            cryptoStream->set_final_byte(this->lastByteOfEncryptionHeader());
            bool hasCorrectPassword = cryptoStream->prepare_for_decryption();
            
            // set it here, because in case the hasCorrectPassword is false
            // the method CloseDecompressionStream() will properly delete the stream
            intermediateStream = _encryptionStream = cryptoStream;
            
            if (!hasCorrectPassword) {
                this->closeDecompressionStream();
                return nullptr;
            }
        }
        
        if (needsDecompress) {
            ICompressionMethod::Ptr zipMethod = ZipMethodResolver::GetZipMethodInstance(this->compressionMethod());
            
            if (zipMethod != nullptr) {
                intermediateStream = _compressionStream = std::make_shared<compression_decoder_stream>(
                        zipMethod->GetDecoder(), zipMethod->GetDecoderProperties(), *intermediateStream);
            }
        }
    }
    
    return intermediateStream.get();
}

bool ZipArchiveEntry::isRawStreamOpened() const {
    return _rawStream != nullptr;
}

bool ZipArchiveEntry::isDecompressionStreamOpened() const {
    return _compressionStream != nullptr;
}

void ZipArchiveEntry::closeRawStream() {
    _rawStream.reset();
}

void ZipArchiveEntry::closeDecompressionStream() {
    _compressionStream.reset();
    _encryptionStream.reset();
    _archiveStream.reset();
    _immediateBuffer.reset();
}

bool ZipArchiveEntry::setCompressionStream(std::istream& stream,
                                           ICompressionMethod::Ptr method /* = DeflateMethod::Create() */,
                                           CompressionMode mode /* = CompressionMode::Deferred */) {
    // if _inputStream is set, we already have some stream to compress
    // so we discard it
    if (_inputStream != nullptr) {
        this->unloadCompressionData();
    }
    
    _isNewOrChanged = true;
    
    _inputStream = &stream;
    _compressionMethod = method;
    _compressionMode = mode;
    this->setCompressionMethod(method->GetZipMethodDescriptor().GetCompressionMethod());
    
    if (_inputStream != nullptr && _compressionMode == CompressionMode::Immediate) {
        _immediateBuffer = std::make_shared<std::stringstream>();
        this->internalCompressStream(*_inputStream, *_immediateBuffer);
        
        // we have everything we need, let's act like we were loaded from archive :)
        _isNewOrChanged = false;
        _inputStream = nullptr;
    }
    
    return true;
}

void ZipArchiveEntry::unSetCompressionStream() {
    if (!this->hasCompressionStream()) {
        this->fetchLocalFileHeader();
    }
    
    this->unloadCompressionData();
    this->setPassword(std::string());
}

void ZipArchiveEntry::remove() {
    auto it = std::find(_archive->_entries.begin(), _archive->_entries.end(), this->shared_from_this());
    
    if (it != _archive->_entries.end()) {
        _archive->_entries.erase(it);
        delete this;
    }
}

//////////////////////////////////////////////////////////////////////////
// private getters & setters

void ZipArchiveEntry::setCompressionMethod(uint16_t value) {
    _centralDirectoryFileHeader.compressionMethod = value;
}

ZipArchiveEntry::BitFlag ZipArchiveEntry::generalPurposeBitFlag() const {
    return static_cast<BitFlag>(_centralDirectoryFileHeader.generalPurposeBitFlag);
}

void ZipArchiveEntry::setGeneralPurposeBitFlag(BitFlag value, bool set) {
    if (set) {
        _centralDirectoryFileHeader.generalPurposeBitFlag |= static_cast<uint16_t>(value);
    } else {
        _centralDirectoryFileHeader.generalPurposeBitFlag &= static_cast<uint16_t>(~value);
    }
}

uint16_t ZipArchiveEntry::versionToExtract() const {
    return _centralDirectoryFileHeader.versionNeededToExtract;
}

void ZipArchiveEntry::setVersionToExtract(uint16_t value) {
    _centralDirectoryFileHeader.versionNeededToExtract = value;
}

uint16_t ZipArchiveEntry::versionMadeBy() const {
    return _centralDirectoryFileHeader.versionMadeBy;
}

void ZipArchiveEntry::setVersionMadeBy(uint16_t value) {
    _centralDirectoryFileHeader.versionMadeBy = value;
}

int32_t ZipArchiveEntry::offsetOfLocalHeader() const {
    return _centralDirectoryFileHeader.relativeOffsetOfLocalHeader;
}

void ZipArchiveEntry::setOffsetOfLocalHeader(int32_t value) {
    _centralDirectoryFileHeader.relativeOffsetOfLocalHeader = static_cast<int32_t>(value);
}

bool ZipArchiveEntry::hasCompressionStream() const {
    return _inputStream != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// private working methods

void ZipArchiveEntry::fetchLocalFileHeader() {
    if (!_hasLocalFileHeader && _originallyInArchive && _archive != nullptr) {
        _archive->zipStream->seekg(this->offsetOfLocalHeader(), std::ios::beg);
        _localFileHeader.deserialize(*_archive->zipStream);
        
        _offsetOfCompressedData = _archive->zipStream->tellg();
    }
    
    // sync data
    this->syncLFH_with_CDFH();
    _hasLocalFileHeader = true;
}

void ZipArchiveEntry::checkFilenameCorrection() {
    // this forces recheck of the filename.
    // this is useful when the check is needed after
    // deserialization
    this->setFullName(this->fullName());
}

void ZipArchiveEntry::fixVersionToExtractAtLeast(uint16_t value) {
    if (this->versionToExtract() < value) {
        this->setVersionToExtract(value);
    }
}

void ZipArchiveEntry::syncLFH_with_CDFH() {
    _localFileHeader.syncWithCentralDirectoryFileHeader(_centralDirectoryFileHeader);
}

void ZipArchiveEntry::syncCDFH_with_LFH() {
    _centralDirectoryFileHeader.syncWithLocalFileHeader(_localFileHeader);
    
    this->fixVersionToExtractAtLeast(this->isDirectory()
                                     ? VERSION_NEEDED_EXPLICIT_DIRECTORY
                                     : _compressionMethod->GetZipMethodDescriptor().GetVersionNeededToExtract());
}

std::ios::pos_type ZipArchiveEntry::offsetOfCompressedData() {
    if (!_hasLocalFileHeader) {
        this->fetchLocalFileHeader();
    }
    
    return _offsetOfCompressedData;
}

std::ios::pos_type ZipArchiveEntry::seekToCompressedData() {
    // check for fail bit?
    _archive->zipStream->seekg(this->offsetOfCompressedData(), std::ios::beg);
    return this->offsetOfCompressedData();
}

void ZipArchiveEntry::serializeLocalFileHeader(std::ostream& stream) {
    // ensure opening the stream
    std::istream* compressedDataStream = nullptr;
    
    if (!this->isDirectory()) {
        if (_inputStream == nullptr) {
            if (!_isNewOrChanged) {
                // the file was either compressed in immediate mode,
                // or was in previous archive
                compressedDataStream = this->rawStream();
            }
            
            // if file is new and empty or stream has been set to nullptr,
            // just do not set any compressed data stream
        } else {
            assert(_isNewOrChanged);
            compressedDataStream = _inputStream;
        }
    }
    
    if (!_hasLocalFileHeader) {
        this->fetchLocalFileHeader();
    }
    
    // save offset of stream here
    _offsetOfSerializedLocalFileHeader = stream.tellp();
    
    if (this->isUsingDataDescriptor()) {
        _localFileHeader.compressedSize = 0;
        _localFileHeader.unCompressedSize = 0;
        _localFileHeader.crc32 = 0;
    }
    
    _localFileHeader.serialize(stream);
    
    // if this entry is a directory, it should not contain any data
    // nor crc.
    assert(
            this->isDirectory()
            ? !crc32() && !size() && !compressedSize() && !_inputStream
            : true
    );
    
    if (!this->isDirectory() && compressedDataStream != nullptr) {
        if (_isNewOrChanged) {
            this->internalCompressStream(*compressedDataStream, stream);
            
            if (this->isUsingDataDescriptor()) {
                _localFileHeader.serializeAsDataDescriptor(stream);
            } else {
                // actualize local file header
                // make non-seekable version?
                stream.seekp(_offsetOfSerializedLocalFileHeader);
                _localFileHeader.serialize(stream);
                stream.seekp(this->compressedSize(), std::ios::cur);
            }
        } else {
            utils::stream::copy(*compressedDataStream, stream);
        }
    }
}

void ZipArchiveEntry::serializeCentralDirectoryFileHeader(std::ostream& stream) {
    _centralDirectoryFileHeader.relativeOffsetOfLocalHeader = static_cast<int32_t>(_offsetOfSerializedLocalFileHeader);
    _centralDirectoryFileHeader.serialize(stream);
}

void ZipArchiveEntry::unloadCompressionData() {
    // unload stream
    _immediateBuffer->clear();
    _inputStream = nullptr;
    
    _centralDirectoryFileHeader.compressedSize = 0;
    _centralDirectoryFileHeader.unCompressedSize = 0;
    _centralDirectoryFileHeader.crc32 = 0;
}

void ZipArchiveEntry::internalCompressStream(std::istream& inputStream, std::ostream& outputStream) {
    std::ostream* intermediateStream = &outputStream;
    
    std::unique_ptr<zip_cryptostream> cryptoStream;
    if (!_password.empty()) {
        this->setGeneralPurposeBitFlag(BitFlag::Encrypted);
        
        cryptoStream = std::make_unique<zip_cryptostream>();
        
        cryptoStream->init(outputStream, _password.c_str());
        cryptoStream->set_final_byte(this->lastByteOfEncryptionHeader());
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
    
    _localFileHeader.unCompressedSize = static_cast<u32>(compressionStream.get_bytes_read());
    _localFileHeader.compressedSize = static_cast<u32>(compressionStream.get_bytes_written() +
                                                       (!_password.empty() ? 12 : 0));
    _localFileHeader.crc32 = crc32Stream.get_crc32();
    
    this->syncCDFH_with_LFH();
}

void ZipArchiveEntry::figureCrc32() {
    if (this->isDirectory() || _inputStream == nullptr || !_isNewOrChanged) {
        return;
    }
    
    // stream must be seekable
    auto position = _inputStream->tellg();
    
    // compute crc32
    crc32stream crc32Stream;
    crc32Stream.init(*_inputStream);
    
    // just force to read all from crc32stream
    nullstream nulldev;
    utils::stream::copy(crc32Stream, nulldev);
    
    // seek back
    _inputStream->clear();
    _inputStream->seekg(position);
    
    _centralDirectoryFileHeader.crc32 = crc32Stream.get_crc32();
}

uint8_t ZipArchiveEntry::lastByteOfEncryptionHeader() {
    if (!!(this->generalPurposeBitFlag() & BitFlag::DataDescriptor)) {
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
        return uint8_t((_centralDirectoryFileHeader.lastModificationTime >> 8) & 0xff);
    } else {
        // When bit 3 is not set, the CRC value is required before
        // encryption of the file data begins. In this case there is no way
        // around it: must read the stream in its entirety to compute the
        // actual CRC before proceeding.
        this->figureCrc32();
        return uint8_t((this->crc32() >> 24) & 0xff);
    }
}
