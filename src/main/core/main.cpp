//
// Created by Khyber on 1/8/2019.
//

#include "src/main/core/main.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "src/lib/fs/fs.h"
#include "src/lib/json/using/Json.h"
#include "src/lib/zip/ZipFile.h"

//#include "llvm/IR/Value.h"

Json parse(const fs::path& path) {
    std::ifstream file(path);
    Json json;
    file >> json;
    return std::move(json);
}

int main() {
    const auto archive = ZipFile::open("/mnt/c/Users/Khyber/Downloads/Maze Starter.sb3");
    std::cout << "numEntries: " << archive.entries().size() << std::endl;
    for (const auto& entry : archive) {
        const auto& e = *entry;
        std::cout << e.name() << std::endl;
        e.remove();
    }
    
    const Json json = parse("/mnt/c/Users/Khyber/Downloads/project.json");
    std::cout << "json.size(): " << json.size() << std::endl;
    std::cout << "costumes: " << std::setw(4) << json["costumes"] << std::endl;
//    llvm::Value* value = nullptr;
//    std::cout << value << std::endl;
    return EXIT_SUCCESS;
}