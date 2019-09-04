//
// Created by Khyber on 1/22/2019.
//

#include "modifiedZipLib.h"

#include <chrono>
#include <iostream>
#include <ios>
#include <fstream>

#include "src/lib/zip/ZipArchive.h"

void modifiedZipLib() {
    using namespace std::chrono_literals;
    
    std::cout << std::boolalpha;
    
    auto archive = ZipArchive("/mnt/c/Users/Khyber/Downloads/Maze Starter.sb3");
    std::this_thread::sleep_for(10ms);
    
    // TODO make decompression const, since it doesn't change the archive
    auto& entry = archive.entry("project.json").get();
    std::cout << "stream" << std::endl;
    auto& stream = *entry.decompressionStream();
    std::cout << stream.fail() << std::endl;
    std::cout << stream.tellg() << std::endl;
    std::cout << entry.fullName() << std::endl;
//    char chars[1000] = {};
//    stream.read(chars, sizeof(chars) - 1);
//    std::string s(chars);
//    std::cout << "start: " << s << " (" << s.size() << ")" << std::endl;
    
    std::ofstream out("/mnt/c/Users/Khyber/Downloads/project.silicon.json");
    out << "Hello" << std::endl;
    std::copy_n(std::istreambuf_iterator(stream),
                1000,
                std::ostreambuf_iterator(out));
//    stream >> std::cout;

//    std::cout << "json" << std::endl;
//    Json json;
//    std::cout << "deserialize" << std::endl;
//    stream >> json;
//    std::cout << "print" << std::endl;
//    std::cout << "json.size(): " << json.size() << std::endl;
//    std::cout << "costumes: " << std::setw(4) << json["costumes"] << std::endl;
}