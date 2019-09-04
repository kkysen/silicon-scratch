//
// Created by Khyber on 1/22/2019.
//

#include "originalZipLib.h"

#include <fstream>

#include "ZipLib/ZipFile.h"

//void originalZipLib() {
//    auto archive = ZipFile::Open("/mnt/c/Users/Khyber/Downloads/Maze Starter.sb3");
//    auto entry = archive->GetEntry("project.json");
//    auto& stream = *entry->GetDecompressionStream();
//
//    std::ofstream out("/mnt/c/Users/Khyber/Downloads/project.silicon.json");
//    out << "Hello" << std::endl;
//    std::copy_n(std::istreambuf_iterator(stream),
//                1000,
//                std::ostreambuf_iterator(out));
//}

//int main() {
//    originalZipLib();
//}