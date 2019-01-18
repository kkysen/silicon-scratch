#pragma once

#include "detail/ZipLocalFileHeader.h"
#include "detail/ZipCentralDirectoryFileHeader.h"

#include "methods/ICompressionMethod.h"
#include "methods/StoreMethod.h"
#include "methods/DeflateMethod.h"
#include "methods/LzmaMethod.h"

#include "streams/substream.h"
#include "utils/enum_utils.h"

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>
#include <memory>

class ZipArchive;

/**
 * \brief Represents a compressed file within a zip archive.
 */
class ZipArchiveEntry : public std::enable_shared_from_this<ZipArchiveEntry> {
    
    friend class ZipArchive;

public:
    
    enum class CompressionMode {
        Immediate,
        Deferred
    };
    
    /**
     * \brief Values that represent the MS-DOS file attributes.
     */
    enum class Attributes : u32 {
        None = 0,
        ReadOnly = 1,
        Hidden = 2,
        System = 4,
        Directory = 16,
        Archive = 32,
        Device = 64,
        Normal = 128,
        Temporary = 256,
        SparseFile = 512,
        ReparsePoint = 1024,
        Compressed = 2048,
    };
    
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(Attributes);
    
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(CompressionMode);

private:
    
    ZipArchive& archive;           //< pointer to the owning zip archive
    
    std::shared_ptr<std::istream> _rawStream = nullptr;         //< stream of raw compressed data
    std::shared_ptr<std::istream> compressionStream = nullptr; //< stream of uncompressed data
    std::shared_ptr<std::istream> encryptionStream = nullptr;  //< underlying encryption stream
    std::shared_ptr<std::istream> archiveStream = nullptr;     //< substream of owning zip archive file
    
    // internal compression data
    std::shared_ptr<std::iostream> immediateBuffer;   //< stream used in the immediate mode, stores compressed data in memory
    std::istream* inputStream = nullptr;       //< input stream
    
    ICompressionMethod::Ptr _compressionMethod; //< compression method
    CompressionMode _compressionMode = CompressionMode::Immediate;   //< compression mode, either deferred or immediate
    
    std::string _name;
    
    // TODO: make as flags
    bool originallyInArchive = false;
    bool isNewOrChanged = false;
    bool hasLocalFileHeader = false;
    
    using Local = detail::ZipLocalFileHeader;
    using Central = detail::ZipCentralDirectoryFileHeader;
    
    struct {
        Local local;
        Central central;
    } fileHeader;
    
    struct {
        std::ios::pos_type compressedData;
        std::ios::pos_type serializedLocalFileHeader;
    } offset = {
            .compressedData = -1,
            .serializedLocalFileHeader = -1,
    };
    
    std::string _password;

public:
    
    /**
     * \brief Destructor.
     */
    ~ZipArchiveEntry();
    
    /**
     * \brief Gets full path of the entry.
     *
     * \return  The full name with the path.
     */
    std::string_view fullName() const noexcept;
    
    /**
     * \brief Sets full name with the path of the entry.
     *
     * \param fullName The full name with the path.
     */
    void setFullName(std::string_view fullName);
    
    /**
     * \brief Gets only the file name of the entry (without path).
     *
     * \return  The file name.
     */
    std::string_view name() const noexcept;
    
    /**
     * \brief Sets only a file name of the entry.
     *        If the file is located within some folder, the path is kept.
     *
     * \param name  The file name.
     */
    void setName(std::string_view name);
    
    /**
     * \brief Gets the comment of this zip entry.
     *
     * \return  The comment.
     */
    std::string_view comment() const noexcept;
    
    /**
     * \brief Sets a comment of this zip entry.
     *
     * \param comment The comment.
     */
    void setComment(std::string_view comment);
    
    /**
     * \brief Gets the time the file was last modified.
     *
     * \return  The last write time.
     */
    time_t lastWriteTime() const noexcept;
    
    /**
     * \brief Sets the time the file was last modified.
     *
     * \param modTime Time of the modifier.
     */
    void setLastWriteTime(time_t modTime) noexcept;
    
    /**
     * \brief Gets the file attributes of this zip entry.
     *
     * \return  The file attributes.
     */
    Attributes attributes() const noexcept;
    
    /**
     * \brief Sets the file attributes of this zip entry.
     *
     * \param value The file attributes.
     */
    void setAttributes(Attributes value);
    
    /**
     * \brief Gets the compression method.
     *
     * \return  The compression method.
     */
    const u16& compressionMethod() const noexcept;

private:
    
    u16& compressionMethod() noexcept;

public:
    
    /**
     * \brief Query if this entry is password protected.
     *
     * \return  true if password protected, false if not.
     */
    bool isPasswordProtected() const noexcept;
    
    /**
     * \brief Gets the password of the zip entry. If the password is empty string, the password is not set.
     *
     * \return  The password.
     */
    std::string_view password() const noexcept;
    
    /**
     * \brief Sets a password of the zip entry. If the password is empty string, the password is not set.
     *        Use before GetDecompressionStream or SetCompressionStream.
     *
     * \param password  The password.
     */
    void setPassword(std::string_view password);
    
    /**
     * \brief Gets CRC 32 of the file.
     *
     * \return  The CRC 32.
     */
    u32 crc32() const noexcept;
    
    /**
     * \brief Gets the size of the uncompressed data.
     *
     * \return  The size.
     */
    size_t size() const noexcept;
    
    /**
     * \brief Gets the size of compressed data.
     *
     * \return  The compressed size.
     */
    size_t compressedSize() const noexcept;
    
    /**
     * \brief Determine if we can extract the entry.
     *        It depends on which version was the zip archive created with.
     *
     * \return  true if we can extract, false if not.
     */
    bool canExtract() const noexcept;
    
    /**
     * \brief Query if this entry is a directory.
     *
     * \return  true if directory, false if not.
     */
    bool isDirectory() const noexcept;
    
    /**
     * \brief Query if this object is using data descriptor.
     *        Data descriptor is small chunk of information written after the compressed data.
     *        It's most useful when encrypting a zip entry.
     *        When it is not using, the CRC32 value is required before
     *        encryption of the file data begins. In this case there is no way
     *        around it: must read the stream in its entirety to compute the
     *        actual CRC32 before proceeding.
     *
     * \return  true if using data descriptor, false if not.
     */
    bool isUsingDataDescriptor() const noexcept;
    
    /**
     * \brief Use data descriptor.
     *        Data descriptor is small chunk of information written after the compressed data.
     *        It's most useful when encrypting a zip entry.
     *        When it is not using, the CRC32 value is required before
     *        encryption of the file data begins. In this case there is no way
     *        around it: must read the stream in its entirety to compute the
     *        actual CRC32 before proceeding.
     * \param use (Optional) If true, use the data descriptor, false to not use.
     */
    void useDataDescriptor(bool use = true) noexcept;
    
    /**
     * \brief Sets the input stream to fetch the data to compress from.
     *
     * \param stream  The input stream to compress.
     * \param method  (Optional) The method of compression.
     * \param mode    (Optional) The mode of compression.
     *                If deferred mode is chosen, the data are compressed when the zip archive is about to be written.
     *                The stream instance must exist when the ZipArchive::WriteToStream method is called.
     *                The advantage of deferred compression mode is the compressed data needs not to be loaded
     *                into the memory, because they are streamed into the final output stream.
     *
     *                If immediate mode is chosen, the data are compressed immediately into the memory buffer.
     *                It is not recommended to use this method for large files.
     *                The advantage of immediate mode is the input stream can be destroyed (i.e. by scope)
     *                even before the ZipArchive::WriteToStream method is called.
     *
     * \return  true if it succeeds, false if it fails.
     */
    bool setCompressionStream(std::istream& stream, ICompressionMethod::Ptr method = DeflateMethod::Create(),
                              CompressionMode mode = CompressionMode::Deferred);
    
    /**
     * \brief Sets compression stream to be null and unsets the password. The entry would contain no data with zero size.
     */
    void unSetCompressionStream();
    
    /**
     * \brief Gets raw stream of the compressed data.
     *
     * \return  null if it fails, else the stream of raw data.
     */
    std::istream* rawStream();
    
    /**
     * \brief Gets decompression stream.
     *        If the file is encrypted and correct password is not provided, it returns nullptr.
     *
     * \return  null if it fails, else the decompression stream.
     */
    std::istream* decompressionStream();
    
    /**
     * \brief Query if the GetRawStream method has been already called.
     *
     * \return  true if the raw stream is opened, false if not.
     */
    bool isRawStreamOpened() const noexcept;
    
    /**
     * \brief Query if the GetDecompressionStream method has been already called.
     *
     * \return  true if the decompression stream is opened, false if not.
     */
    bool isDecompressionStreamOpened() const noexcept;
    
    /**
     * \brief Closes the raw stream, opened by GetRawStream.
     */
    void closeRawStream();
    
    /**
     * \brief Closes the decompression stream, opened by GetDecompressionStream.
     */
    void closeDecompressionStream();
    
    /**
     * \brief Removes this entry from the ZipArchive.
     */
    void remove();

private:
    
    static constexpr u16 VERSION_MADE_BY_DEFAULT = 63;
    static constexpr u16 VERSION_NEEDED_DEFAULT = 10;
    static constexpr u16 VERSION_NEEDED_EXPLICIT_DIRECTORY = 20;
    
    static constexpr u16 VERSION_NEEDED_ZIP64 = 45;
    
    enum class BitFlag : u16 {
        None = 1 << 0,
        Encrypted = 1 << 1,
        DataDescriptor = 1 << 3,
        UnicodeFileName = 1 << 11,
    };
    
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(BitFlag);
    
public:
    
    ZipArchiveEntry(ZipArchive& archive, std::string_view fullPath);
    
    ZipArchiveEntry(ZipArchive& archive, Central& central);

private:
    
    const BitFlag& generalPurposeBitFlag() const noexcept;
    
    BitFlag& generalPurposeBitFlag() noexcept;
    
    void setGeneralPurposeBitFlag(BitFlag flag, bool set = true) noexcept;
    
    const u16& versionToExtract() const noexcept;
    
    u16& versionToExtract() noexcept;
    
    const u16& versionMadeBy() const noexcept;
    
    u16& versionMadeBy() noexcept;
    
    const i32& offsetOfLocalHeader() const noexcept;
    
    i32& offsetOfLocalHeader() noexcept;
    
    bool hasCompressionStream() const noexcept;
    
    void fetchLocalFileHeader();
    
    void checkFileNameCorrection();
    
    void fixVersionToExtractAtLeast(u16 value);
    
    void syncLocalWithCentralDirectoryFileHeader();
    
    void syncCentralDirectoryWithLocalFileHeader();
    
    std::ios::pos_type offsetOfCompressedData();
    
    std::ios::pos_type seekToCompressedData();
    
    void serializeLocalFileHeader(std::ostream& stream);
    
    void serializeCentralDirectoryFileHeader(std::ostream& stream);
    
    void unloadCompressionData();
    
    void internalCompressStream(std::istream& inputStream, std::ostream& outputStream);
    
    // for encryption
    void figureCrc32();
    
    u32 lastUintOfEncryptionHeader();
    
    u8 lastByteOfEncryptionHeader();
    
};
