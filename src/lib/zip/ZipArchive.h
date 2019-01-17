#pragma once

#include "src/lib/zip/detail/EndOfCentralDirectoryBlock.h"

#include "ZipArchiveEntry.h"

#include <istream>
#include <vector>
#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <variant>

#include "src/main/util/numbers.h"

/**
 * \brief Represents a package of compressed files in the zip archive format.
 */
class ZipArchive : public std::enable_shared_from_this<ZipArchive> {
    
    friend class ZipArchiveEntry;

public:
    
    using Ptr = std::shared_ptr<ZipArchive>;
    
    using Entries = std::vector<ZipArchiveEntry::Ptr>;
    using ConstIterator = typename Entries::const_iterator;
    using Iterator = typename Entries::iterator;

private:
    
    detail::EndOfCentralDirectoryBlock endOfCentralDirectoryBlock;
    Entries _entries;
    std::istream* zipStream = nullptr;
    bool ownsStream = false;

public:
    
    const std::string& comment() const noexcept;
    
    std::string& comment() noexcept;
    
    const std::vector<ZipArchiveEntry::Ptr>& entries() const noexcept;
    
    Entries& entries() noexcept;
    
    ConstIterator begin() const noexcept;
    
    Iterator begin() noexcept;
    
    ConstIterator end() const noexcept;
    
    Iterator end() noexcept;

private:
    
    ConstIterator findEntry(std::string_view name) const noexcept;

public:
    
    class MaybeEntry;
    
    class ConstMaybeEntry {
        
        friend MaybeEntry;

    private:
        
        const std::shared_ptr<const ZipArchive> archive;
        std::variant<ConstIterator, std::string, char> variant = 0;
        // store either entry, which has a name, or name for creation

    public:
        
        ConstMaybeEntry(std::shared_ptr<const ZipArchive> archive, const std::string& name) noexcept;

    private:
        
        const std::string& directName() const noexcept;
    
        ConstIterator iterator() const noexcept;
    
        std::shared_ptr<const ZipArchiveEntry> entryPtr() const noexcept;
    
        const ZipArchiveEntry& entry() const noexcept;
    
        const std::string& entryName() const noexcept;
    
        std::runtime_error error(std::string_view action, std::string_view reason) const noexcept;
        
    public:
    
        bool exists() const noexcept;
    
        operator bool() const noexcept;
        
        const std::string& name() const noexcept;
        
        const ZipArchiveEntry& get() const;
        
    };
    
    class MaybeEntry {
    
    private:
        
        ConstMaybeEntry impl;

    public:
    
        MaybeEntry(std::shared_ptr<ZipArchive> archive, const std::string& name) noexcept;

    private:
        
        ZipArchive& archive() noexcept;
    
        Iterator iterator() noexcept;
    
        std::shared_ptr<ZipArchiveEntry> entryPtr() noexcept;
        
    public:
    
        bool exists() const noexcept;
        
        operator bool() const noexcept;
    
        const std::string& name() const noexcept;
        
        std::string& name() noexcept;
        
        const ZipArchiveEntry& get() const;
        
        ZipArchiveEntry& get();

    private:

        void createForcefully();
        
        void removeForcefully();
        
    public:
        
        enum class CreateMode { IF_NOT_EXISTS, OVERWRITE, FAIL_IF_EXISTS };
        
        MaybeEntry& create(CreateMode mode);
        
        enum class RemoveMode { IF_EXISTS, FAIL_IF_NOT_EXISTS };
        
        MaybeEntry& remove(RemoveMode mode);
        
    };
    
    ConstMaybeEntry entry(const std::string& name) const noexcept;
    
    MaybeEntry entry(const std::string& name) noexcept;

private:
    
    enum class SeekDirection {
        Forward,
        Backward
    };
    
    bool seekToSignature(u32 signature, SeekDirection direction);

    bool readEndOfCentralDirectory();
    
    bool ensureCentralDirectoryRead();
    
    bool init();
    
public:
    
    ZipArchive() = default;
    
    ZipArchive(ZipArchive&& other) noexcept;
    
    explicit ZipArchive(std::istream& stream);
    
    ZipArchive(std::istream* stream, bool takeOwnership);
    
    ZipArchive(const ZipArchive& other) = delete;
    
    ZipArchive& operator=(const ZipArchive& other) = delete;
    
    ZipArchive& operator=(ZipArchive&& other) noexcept = delete;
    
    ~ZipArchive();
    
    void writeTo(std::ostream& out);
    
};

std::ostream& operator<<(ZipArchive& archive, std::ostream& out);
