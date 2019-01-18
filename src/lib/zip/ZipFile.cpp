#include <utility>

#include <utility>

#include "ZipFile.h"

#include "utils/stream_utils.h"

#include <fstream>

namespace {
    
    std::string getFileNameFromPath(const std::string& fullPath) {
        std::string::size_type dirSeparatorPos;
        
        if ((dirSeparatorPos = fullPath.find_last_of('/')) != std::string::npos) {
            return fullPath.substr(dirSeparatorPos + 1);
        } else {
            return fullPath;
        }
    }
    
    std::string makeTempFilename(const std::string& fileName) {
        return fileName + ".tmp";
    }
    
}

ZipArchive ZipFile::open(const std::string& zipPath) {
    auto zipFile = std::make_unique<std::ifstream>();
    zipFile->open(zipPath, std::ios::binary);
    
    if (!zipFile->is_open()) {
        // if file does not exist, try to create it
        std::ofstream tmpFile;
        tmpFile.open(zipPath, std::ios::binary);
        tmpFile.close();
        
        zipFile->open(zipPath, std::ios::binary);
        
        // if attempt to create file failed, throw an exception
        if (!zipFile->is_open()) {
            throw std::runtime_error("cannot open zip file");
        }
    }
    return std::move(ZipArchive(zipFile.release(), true));
}

void ZipFile::AddFile(
        const std::string& zipPath, const std::string& fileName, ICompressionMethod::Ptr method) {
    AddFile(zipPath, fileName, getFileNameFromPath(fileName), std::move(method));
}

void ZipFile::AddFile(
        const std::string& zipPath, const std::string& fileName, const std::string& inArchiveName,
        ICompressionMethod::Ptr method) {
    AddEncryptedFile(zipPath, fileName, inArchiveName, std::string(), std::move(method));
}

void ZipFile::AddEncryptedFile(
        const std::string& zipPath, const std::string& fileName, const std::string& password,
        ICompressionMethod::Ptr method) {
    // not sure why empty string was being passed as the password, so I changed it
    AddEncryptedFile(zipPath, fileName, getFileNameFromPath(fileName), password, std::move(method));
//    AddEncryptedFile(zipPath, fileName, GetFilenameFromPath(fileName), std::string(), std::move(method));
}

void ZipFile::AddEncryptedFile(
        const std::string& zipPath, const std::string& fileName, const std::string& inArchiveName,
        const std::string& password, ICompressionMethod::Ptr method) {
    std::string tmpName = makeTempFilename(zipPath);
    
    {
        auto zipArchive = ZipFile::open(zipPath);
        
        std::ifstream fileToAdd;
        fileToAdd.open(fileName, std::ios::binary);
        
        if (!fileToAdd.is_open()) {
            throw std::runtime_error("cannot open input file");
        }
        
        auto& fileEntry = zipArchive
                .entry(inArchiveName)
                .create(ZipArchive::MaybeEntry::CreateMode::OVERWRITE)
                .get();
        
        if (!password.empty()) {
            fileEntry.setPassword(password);
            fileEntry.useDataDescriptor();
        }
        fileEntry.setCompressionStream(fileToAdd, std::move(method));
        
        //////////////////////////////////////////////////////////////////////////
        
        std::ofstream outFile;
        outFile.open(tmpName, std::ios::binary);
        
        if (!outFile.is_open()) {
            throw std::runtime_error("cannot open output file");
        }
        
//        outFile << zipArchive;
        zipArchive.writeTo(outFile);
        outFile.close();
        
        // force closing the input zip stream
    }
    
    remove(zipPath.c_str());
    rename(tmpName.c_str(), zipPath.c_str());
}

void ZipFile::ExtractFile(const std::string& zipPath, const std::string& fileName) {
    ExtractFile(zipPath, fileName, getFileNameFromPath(fileName));
}

void ZipFile::ExtractFile(
        const std::string& zipPath, const std::string& fileName, const std::string& destinationPath) {
    ExtractEncryptedFile(zipPath, fileName, destinationPath, std::string());
}

void ZipFile::ExtractEncryptedFile(
        const std::string& zipPath, const std::string& fileName, const std::string& password) {
    ExtractEncryptedFile(zipPath, fileName, getFileNameFromPath(fileName), password);
}

void ZipFile::ExtractEncryptedFile(const std::string& zipPath, const std::string& fileName,
                                   const std::string& destinationPath, const std::string& password) {
    auto zipArchive = ZipFile::open(zipPath);
    
    std::ofstream destFile;
    destFile.open(destinationPath, std::ios::binary | std::ios::trunc);
    
    if (!destFile.is_open()) {
        throw std::runtime_error("cannot create destination file");
    }
    
    auto& entry = zipArchive.entry(fileName).get();
    
    if (!password.empty()) {
        entry.setPassword(password);
    }
    
    std::istream* dataStream = entry.decompressionStream();
    if (dataStream == nullptr) {
        throw std::runtime_error("wrong password");
    }
    
    utils::stream::copy(*dataStream, destFile);
    
    destFile.flush();
    destFile.close();
}