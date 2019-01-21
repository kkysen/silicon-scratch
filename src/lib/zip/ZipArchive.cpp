#include "ZipArchive.h"

#include <fstream>

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

size_t ZipArchive::size() const noexcept {
    return entries().size();
}

const ZipArchiveEntry& ZipArchive::operator[](size_t i) const noexcept {
    return *entries()[i];
}

ZipArchiveEntry& ZipArchive::operator[](size_t i) noexcept {
    return *entries()[i];
}


size_t ZipArchive::findEntry(std::string_view name) const noexcept {
    size_t i = 0;
    for (const auto& entry : *this) {
        if (entry.fullName() == name) {
            return i;
        }
        i++;
    }
    return invalidIndex;
//    const auto it = std::find_if(begin(), end(), [&name](const ZipArchiveEntry& entry) {
//        return entry.fullName() == name;
//    });
//    if (it == end()) {
//        return invalidIndex;
//    }
//    return static_cast<size_t>(end() - it);
}

void ZipArchive::removeEntry(size_t index) {
    // TODO use SlicedIterable
    for (auto it = begin() + index; it != end(); ++it) {
        it->index--;
    }
    entries().erase(entries().begin() + index);
}


ZipArchive::ConstMaybeEntry::ConstMaybeEntry(const ZipArchive& archive,
                                             const std::string& name) noexcept
        : archive(std::move(archive)) {
    const auto index = archive.findEntry(name);
    if (index == invalidIndex) {
        variant = name;
    } else {
        variant = index;
    }
}

std::string_view ZipArchive::ConstMaybeEntry::directName() const noexcept {
    return std::get<std::string>(variant);
}

size_t ZipArchive::ConstMaybeEntry::index() const noexcept {
    return std::get<size_t>(variant);
}

const ZipArchiveEntry& ZipArchive::ConstMaybeEntry::entry() const noexcept {
    return *archive.entries()[index()];
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
    return std::holds_alternative<size_t>(variant);
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


ZipArchive::MaybeEntry::MaybeEntry(ZipArchive& archive, const std::string& name) noexcept
        : impl(archive, name) {}

ZipArchive::Entries& ZipArchive::MaybeEntry::entries() noexcept {
    return const_cast<Entries&>(impl.archive.entries());
}

ZipArchiveEntry& ZipArchive::MaybeEntry::entry() noexcept {
    return const_cast<ZipArchiveEntry&>(impl.entry());
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
    auto& entries = this->entries();
    const auto exists = this->exists();
    const auto i = exists ? impl.index() : entries.size();
    auto newEntry = std::make_unique<ZipArchiveEntry>(ZipArchiveEntry::ConstructorKey(),
                                                      const_cast<ZipArchive&>(impl.archive), i, name());
    if (exists) {
        entries[i].swap(newEntry);
    } else {
        impl.variant = i;
        entries.push_back(std::move(newEntry));
    }
}

void ZipArchive::MaybeEntry::removeForcefully() {
    auto& entry = this->entry();
    impl.variant = std::string(name());
    entry.remove();
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
    return ConstMaybeEntry(*this, name);
}

ZipArchive::MaybeEntry ZipArchive::entry(const std::string& name) noexcept {
    return MaybeEntry(*this, name);
}


bool ZipArchive::seekToSignature(u32 signature, ZipArchive::SeekDirection direction) {
    auto& stream = *this->stream;
    auto streamPosition = stream.tellg();
    while (!stream.eof() && !stream.fail()) {
        u32 buffer;
        deserialize(stream, buffer);
        if (buffer == signature) {
            stream.seekg(streamPosition, std::ios::beg);
            return true;
        }
        streamPosition += static_cast<i32>(direction);
        stream.seekg(streamPosition, std::ios::beg);
    }
    return false;
}

bool ZipArchive::readEndOfCentralDirectory() {
    constexpr auto signature = EndOfCentralDirectoryBlock::constants::signature;
    constexpr auto minShift = (EndOfCentralDirectoryBlock::size - sizeof(signature));
    auto& stream = *this->stream;
    stream.seekg(-minShift, std::ios::end);
    if (seekToSignature(EndOfCentralDirectoryBlock::constants::signature, SeekDirection::Backward)) {
        endOfCentralDirectoryBlock.deserialize(stream);
        return true;
    }
    return false;
}

bool ZipArchive::ensureCentralDirectoryRead() {
    auto& stream = *this->stream;
    stream.seekg(endOfCentralDirectoryBlock.offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber,
                 std::ios::beg);
    auto& entries = this->entries();
    for (;;) {
        ZipCentralDirectoryFileHeader central;
        if (!central.deserialize(stream)) {
            break;
        }
        entries.push_back(std::make_unique<ZipArchiveEntry>(ZipArchiveEntry::ConstructorKey(),
                                                            *this, entries.size(), central));
    }
    return true;
}

bool ZipArchive::init() {
    return readEndOfCentralDirectory() && ensureCentralDirectoryRead();
}


ZipArchive::ZipArchive(std::unique_ptr<std::istream>&& stream) : stream(std::move(stream)) {
    init();
}

ZipArchive::ZipArchive(const fs::path& path) : ZipArchive(std::make_unique<std::ifstream>(path)) {}


void ZipArchive::writeTo(std::ostream& stream) {
    const auto startPosition = stream.tellp();
    
    // TODO make serialization const
    for (auto& entry : *this) {
        entry.serializeLocalFileHeader(stream);
    }
    
    const auto offsetOfStartOfCDFH = stream.tellp() - startPosition;
    for (auto& entry : *this) {
        entry.serializeCentralDirectoryFileHeader(stream);
    }
    
    auto& block = endOfCentralDirectoryBlock;
    
    block.diskNumber = 0;
    block.startOfCentralDirectoryDiskNum = 0;
    
    const auto numEntries = static_cast<u16>(entries().size());;
    block.numberEntriesInCentralDirectory = numEntries;
    block.numberEntriesInDiskCentralDirectory = numEntries;
    
    block.centralDirectorySize = static_cast<u32>(stream.tellp() - offsetOfStartOfCDFH);
    block.offsetOfStartOfCentralDirectoryWithRespectToStartingDiskNumber = static_cast<u32>(offsetOfStartOfCDFH);
    
    block.serialize(stream);
}


std::ostream& operator<<(ZipArchive& archive, std::ostream& out) {
    archive.writeTo(out);
    return out;
}
