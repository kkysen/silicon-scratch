#include <memory>

#include "ZipArchive.h"
#include "streams/serialization.h"

#include "src/main/util/cast/iterators.h"

using detail::EndOfCentralDirectoryBlock;
using detail::ZipCentralDirectoryFileHeader;

std::string_view ZipArchive::comment() const noexcept {
    return endOfCentralDirectoryBlock.comment;
}

void ZipArchive::setComment(std::string_view comment) noexcept {
    endOfCentralDirectoryBlock.comment = comment;
}


const ZipArchive::Entries& ZipArchive::entries() const noexcept {
    return _entries;
}

ZipArchive::Entries& ZipArchive::entries() noexcept {
    return _entries;
}

ZipArchive::ConstIterator ZipArchive::begin() const noexcept {
    return entries().begin();
}

ZipArchive::Iterator ZipArchive::begin() noexcept {
    return entries().begin();
}

ZipArchive::ConstIterator ZipArchive::end() const noexcept {
    return entries().end();
}

ZipArchive::Iterator ZipArchive::end() noexcept {
    return entries().end();
}

ZipArchive::ConstIterator ZipArchive::findEntry(std::string_view name) const noexcept {
    return std::find_if(begin(), end(), [&name](const std::unique_ptr<ZipArchiveEntry>& entry) {
        return entry->fullName() == name;
    });
}


ZipArchive::ConstMaybeEntry::ConstMaybeEntry(std::shared_ptr<const ZipArchive> archivePtr,
                                             const std::string& name) noexcept
        : archive(std::move(archivePtr)) {
    auto& archive = *archivePtr;
    auto found = archive.findEntry(name);
    if (found == archive.end()) {
        variant = name;
    } else {
        variant = found;
    }
}

std::string_view ZipArchive::ConstMaybeEntry::directName() const noexcept {
    return std::get<std::string>(variant);
}

ZipArchive::ConstIterator ZipArchive::ConstMaybeEntry::iterator() const noexcept {
    return std::get<ConstIterator>(variant);
}

const ZipArchiveEntry& ZipArchive::ConstMaybeEntry::entry() const noexcept {
    return **iterator();
}

std::string_view ZipArchive::ConstMaybeEntry::entryName() const noexcept {
    return entry().fullName();
}

std::runtime_error ZipArchive::ConstMaybeEntry::error(std::string_view action, std::string_view reason) const noexcept {
    using namespace std::string_literals;
    std::string message;
    message += "cannot ";
    message += action;
    message += " ZipArchiveEntry ";
    message += name();
    message += ": ";
    message += reason;
    return std::runtime_error(message);
}

bool ZipArchive::ConstMaybeEntry::exists() const noexcept {
    return std::holds_alternative<ConstIterator>(variant);
}

ZipArchive::ConstMaybeEntry::operator bool() const noexcept {
    return exists();
}

std::string_view ZipArchive::ConstMaybeEntry::name() const noexcept {
    return exists() ? directName() : entryName();
}

const ZipArchiveEntry& ZipArchive::ConstMaybeEntry::get() const {
    if (!exists()) {
        throw error("get", "does not exist");
    }
    return entry();
}


ZipArchive::MaybeEntry::MaybeEntry(std::shared_ptr<ZipArchive> archive, const std::string& name) noexcept
        : impl(archive, name) {}

ZipArchive& ZipArchive::MaybeEntry::archive() noexcept {
    return const_cast<ZipArchive&>(*impl.archive);
}

ZipArchive::Iterator ZipArchive::MaybeEntry::iterator() noexcept {
    return iterators::removeConst<Entries>(impl.iterator());
}

std::unique_ptr<ZipArchiveEntry>& ZipArchive::MaybeEntry::entryPtr() noexcept {
    return const_cast<std::unique_ptr<ZipArchiveEntry>&>(*impl.iterator());
}

bool ZipArchive::MaybeEntry::exists() const noexcept {
    return impl.exists();
}

ZipArchive::MaybeEntry::operator bool() const noexcept {
    return exists();
}

std::string_view ZipArchive::MaybeEntry::name() const noexcept {
    return impl.name();
}

void ZipArchive::MaybeEntry::setName(std::string_view name) noexcept {
    if (exists()) {
        const_cast<ZipArchiveEntry&>(impl.entry()).setName(name);
    } else {
        impl.variant = std::string(name);
    }
}

const ZipArchiveEntry& ZipArchive::MaybeEntry::get() const {
    return impl.get();
}


ZipArchiveEntry& ZipArchive::MaybeEntry::get() {
    return const_cast<ZipArchiveEntry&>(impl.get());
}

void ZipArchive::MaybeEntry::createForcefully() {
    auto newEntry = std::make_unique<ZipArchiveEntry>(archive(), name());
    if (exists()) {
        entryPtr().swap(newEntry);
    } else {
        auto& entries = archive().entries();
        impl.variant = entries.end();
        entries.push_back(std::move(newEntry));
    }
}

void ZipArchive::MaybeEntry::removeForcefully() {
    auto& entries = archive().entries();
    entries.erase(iterator());
    impl.variant = std::string(name());
}

ZipArchive::MaybeEntry& ZipArchive::MaybeEntry::create(ZipArchive::MaybeEntry::CreateMode mode) {
    switch (mode) {
        case CreateMode::IF_NOT_EXISTS:
            if (!exists()) {
                createForcefully();
            }
            break;
        case CreateMode::OVERWRITE:
            createForcefully();
            break;
        case CreateMode::FAIL_IF_EXISTS:
            if (exists()) {
                throw impl.error("create", "already exists");
            }
            createForcefully();
            break;
    }
    return *this;
}

ZipArchive::MaybeEntry& ZipArchive::MaybeEntry::remove(ZipArchive::MaybeEntry::RemoveMode mode) {
    switch (mode) {
        case RemoveMode::IF_EXISTS:
            if (exists()) {
                removeForcefully();
            }
            break;
        case RemoveMode::FAIL_IF_NOT_EXISTS:
            if (!exists()) {
                throw impl.error("remove", "does not exist");
            }
            removeForcefully();
            break;
    };
    return *this;
}


ZipArchive::ConstMaybeEntry ZipArchive::entry(const std::string& name) const noexcept {
    return ConstMaybeEntry(shared_from_this(), name);
}

ZipArchive::MaybeEntry ZipArchive::entry(const std::string& name) noexcept {
    return MaybeEntry(shared_from_this(), name);
}


bool ZipArchive::seekToSignature(u32 signature, ZipArchive::SeekDirection direction) {
    auto& zip = *zipStream;
    
    auto streamPosition = zip.tellg();
    u32 buffer = 0;
    auto appendix = direction == SeekDirection::Backward ? 0 - 1 : 1;
    
    while (!zip.eof() && !zip.fail()) {
        deserialize(zip, buffer);
        if (buffer == signature) {
            zip.seekg(streamPosition, std::ios::beg);
            return true;
        }
        streamPosition += appendix;
        zip.seekg(streamPosition, std::ios::beg);
    }
    
    return false;
}

bool ZipArchive::readEndOfCentralDirectory() {
    constexpr int EOCDB_SIZE = 22; // sizeof(EndOfCentralDirectoryBlockBase);
    constexpr int SIGNATURE_SIZE = 4;  // sizeof(std::declval<EndOfCentralDirectoryBlockBase>().Signature);
    constexpr int MIN_SHIFT = (EOCDB_SIZE - SIGNATURE_SIZE);
    
    zipStream->seekg(-MIN_SHIFT, std::ios::end);
    if (seekToSignature(EndOfCentralDirectoryBlock::SIGNATURE_CONST, SeekDirection::Backward)) {
        endOfCentralDirectoryBlock.deserialize(*zipStream);
        return true;
    }
    return false;
}

bool ZipArchive::ensureCentralDirectoryRead() {
    zipStream->seekg(endOfCentralDirectoryBlock.offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber,
                     std::ios::beg);
    for (;;) {
        ZipCentralDirectoryFileHeader central;
        if (!central.deserialize(*zipStream)) {
            break;
        }
        _entries.push_back(std::make_unique<ZipArchiveEntry>(*this, central));
    }
    return true;
}

bool ZipArchive::init() {
    return readEndOfCentralDirectory() && ensureCentralDirectoryRead();
}


ZipArchive::ZipArchive(ZipArchive&& other) noexcept {
    endOfCentralDirectoryBlock = other.endOfCentralDirectoryBlock;
    _entries = std::move(other._entries);
    zipStream = other.zipStream;
    ownsStream = other.ownsStream;
    
    other.zipStream = nullptr;
    other.ownsStream = false;
}

ZipArchive::ZipArchive(std::istream& stream) : ZipArchive(&stream, false) {}

ZipArchive::ZipArchive(std::istream* stream, bool takeOwnership) {
    zipStream = stream;
    ownsStream = stream != nullptr ? takeOwnership : false;
    if (stream != nullptr) {
        init();
    }
}

ZipArchive::~ZipArchive() {
    if (ownsStream && zipStream != nullptr) {
        delete zipStream;
        zipStream = nullptr;
    }
}


void ZipArchive::writeTo(std::ostream& stream) {
    const auto startPosition = stream.tellp();
    
    for (auto& entry : entries()) {
        entry->serializeLocalFileHeader(stream);
    }
    
    const auto offsetOfStartOfCDFH = stream.tellp() - startPosition;
    for (auto& entry : entries()) {
        entry->serializeCentralDirectoryFileHeader(stream);
    }
    
    auto& block = endOfCentralDirectoryBlock;
    
    block.diskNumber = 0;
    block.startOfCentralDirectoryDiskNum = 0;
    
    block.numberEntriesInCentralDirectory = static_cast<u16>(entries().size());
    block.numberEntriesInDiskCentralDirectory = static_cast<u16>(entries().size());
    
    block.centralDirectorySize = static_cast<u32>(stream.tellp() - offsetOfStartOfCDFH);
    block.offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber = static_cast<u32>(offsetOfStartOfCDFH);
    
    block.serialize(stream);
}


std::ostream& operator<<(ZipArchive& archive, std::ostream& out) {
    archive.writeTo(out);
    return out;
}
