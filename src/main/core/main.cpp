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

void test1() {
    const auto archive = ZipArchive("/mnt/c/Users/Khyber/Downloads/Maze Starter.sb3");
    for (const auto& entry : archive) {
        std::cout << entry.name() << std::endl;
//        entry.remove();
    }
    
    const Json json = parse("/mnt/c/Users/Khyber/Downloads/project.json");
    std::cout << "json.size(): " << json.size() << std::endl;
    std::cout << "costumes: " << std::setw(4) << json["costumes"] << std::endl;
//    llvm::Value* value = nullptr;
//    std::cout << value << std::endl;
}

int main() {
//    test1();
    using namespace std::chrono_literals;
    
    std::cout << std::boolalpha;
    
    auto archive = ZipArchive("/mnt/c/Users/Khyber/Downloads/Maze Starter.sb3");
    std::this_thread::sleep_for(10ms);
    
    // TODO make decompression const, since it doesn't change the archive
    std::cout << "stream" << std::endl;
    auto& stream = *archive.entry("project.json").get().decompressionStream();
    std::cout << stream.fail() << std::endl;
    std::cout << stream.tellg() << std::endl;
    char chars[1000] = {};
    stream.read(chars, sizeof(chars) - 1);
    std::string s(chars);
    std::cout << s << std::endl;
    
//    std::ofstream out("/mnt/C/Users/Khyber/Downloads/project.silicon.json");
//    std::copy_n(std::istreambuf_iterator(stream),
//              1000,
//              std::ostreambuf_iterator(out));
//    stream >> std::cout;

//    std::cout << "json" << std::endl;
//    Json json;
//    std::cout << "deserialize" << std::endl;
//    stream >> json;
//    std::cout << "print" << std::endl;
//    std::cout << "json.size(): " << json.size() << std::endl;
//    std::cout << "costumes: " << std::setw(4) << json["costumes"] << std::endl;
    
    return EXIT_SUCCESS;
}