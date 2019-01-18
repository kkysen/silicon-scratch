cmake_minimum_required(VERSION 3.9)
project(SiliconScratch CXX)

set(CMAKE_CXX_STANDARD 17)

set(CXX_LIBS /mnt/c/Users/Khyber/workspace/C++)

include_directories(.)
#include_directories(${CXX_LIBS}/ZipLib/source/)
include_directories(${CXX_LIBS}/json/single_include/)
include_directories(${CXX_LIBS}/llvm/llvm/include/)
include_directories(${CXX_LIBS}/emsdk/emscripten/1.38.21/system/include/)

# support lto (link time optimization)
SET(CMAKE_AR "gcc-ar")
SET(CMAKE_RANLIB "gcc-ranlib")


set(CMAKE_C_OPTIMIZATION_FLAGS "-O0 -flto")
set(CMAKE_C_ERROR_FLAGS "-Wall -Werror -Wextra")
set(CMAKE_C_NO_ERROR_FLAGS "-Wno-error=unused-function -Wno-error=unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_ERROR_FLAGS} ${CMAKE_C_NO_ERROR_FLAGS} ${CMAKE_C_OPTIMIZATION_FLAGS}")

set(LIB_FS_FILES
        src/lib/fs/fs.h
        )

set(LIB_JSON_FILES
        src/lib/json/json.h
        src/lib/json/using/Json.h
        )

set(LIB_ZIP_FILES
        src/lib/zip/compression/bzip2/bzip2_decoder.h
        src/lib/zip/compression/bzip2/bzip2_decoder_properties.h
        src/lib/zip/compression/bzip2/bzip2_encoder.h
        src/lib/zip/compression/bzip2/bzip2_encoder_properties.h
        src/lib/zip/compression/compression_interface.h
        src/lib/zip/compression/deflate/deflate_decoder.h
        src/lib/zip/compression/deflate/deflate_decoder_properties.h
        src/lib/zip/compression/deflate/deflate_encoder.h
        src/lib/zip/compression/deflate/deflate_encoder_properties.h
        src/lib/zip/compression/lzma/detail/lzma_alloc.h
        src/lib/zip/compression/lzma/detail/lzma_handle.h
        src/lib/zip/compression/lzma/detail/lzma_header.h
        src/lib/zip/compression/lzma/detail/lzma_in_stream.h
        src/lib/zip/compression/lzma/detail/lzma_out_stream.h
        src/lib/zip/compression/lzma/lzma_decoder.h
        src/lib/zip/compression/lzma/lzma_decoder_properties.h
        src/lib/zip/compression/lzma/lzma_encoder.h
        src/lib/zip/compression/lzma/lzma_encoder_properties.h
        src/lib/zip/compression/store/store_decoder.h
        src/lib/zip/compression/store/store_decoder_properties.h
        src/lib/zip/compression/store/store_encoder.h
        src/lib/zip/compression/store/store_encoder_properties.h
        src/lib/zip/detail/EndOfCentralDirectoryBlock.cpp
        src/lib/zip/detail/EndOfCentralDirectoryBlock.h
        src/lib/zip/detail/ZipCentralDirectoryFileHeader.cpp
        src/lib/zip/detail/ZipCentralDirectoryFileHeader.h
        src/lib/zip/detail/ZipGenericExtraField.cpp
        src/lib/zip/detail/ZipGenericExtraField.h
        src/lib/zip/detail/ZipLocalFileHeader.cpp
        src/lib/zip/detail/ZipLocalFileHeader.h
        src/lib/zip/extlibs/bzip2/blocksort.c
        src/lib/zip/extlibs/bzip2/bzerror.c
        src/lib/zip/extlibs/bzip2/bzlib.c
        src/lib/zip/extlibs/bzip2/bzlib.h
        src/lib/zip/extlibs/bzip2/bzlib_private.h
        src/lib/zip/extlibs/bzip2/compress.c
        src/lib/zip/extlibs/bzip2/crctable.c
        src/lib/zip/extlibs/bzip2/decompress.c
        src/lib/zip/extlibs/bzip2/huffman.c
        src/lib/zip/extlibs/bzip2/randtable.c
        src/lib/zip/extlibs/lzma/unix/7zBuf.h
        src/lib/zip/extlibs/lzma/unix/7zBuf2.c
        src/lib/zip/extlibs/lzma/unix/7zCrc.c
        src/lib/zip/extlibs/lzma/unix/7zCrc.h
        src/lib/zip/extlibs/lzma/unix/7zCrcOpt.c
        src/lib/zip/extlibs/lzma/unix/7zStream.c
        src/lib/zip/extlibs/lzma/unix/7zVersion.h
        src/lib/zip/extlibs/lzma/unix/Aes.c
        src/lib/zip/extlibs/lzma/unix/Aes.h
        src/lib/zip/extlibs/lzma/unix/Alloc.c
        src/lib/zip/extlibs/lzma/unix/Alloc.h
        src/lib/zip/extlibs/lzma/unix/Bra.c
        src/lib/zip/extlibs/lzma/unix/Bra.h
        src/lib/zip/extlibs/lzma/unix/Bra86.c
        src/lib/zip/extlibs/lzma/unix/BraIA64.c
        src/lib/zip/extlibs/lzma/unix/BwtSort.c
        src/lib/zip/extlibs/lzma/unix/BwtSort.h
        src/lib/zip/extlibs/lzma/unix/CpuArch.c
        src/lib/zip/extlibs/lzma/unix/CpuArch.h
        src/lib/zip/extlibs/lzma/unix/Delta.c
        src/lib/zip/extlibs/lzma/unix/Delta.h
        src/lib/zip/extlibs/lzma/unix/HuffEnc.c
        src/lib/zip/extlibs/lzma/unix/HuffEnc.h
        src/lib/zip/extlibs/lzma/unix/LzFind.c
        src/lib/zip/extlibs/lzma/unix/LzFind.h
        src/lib/zip/extlibs/lzma/unix/LzFindMt.c
        src/lib/zip/extlibs/lzma/unix/LzFindMt.h
        src/lib/zip/extlibs/lzma/unix/LzHash.h
        src/lib/zip/extlibs/lzma/unix/Lzma2Dec.c
        src/lib/zip/extlibs/lzma/unix/Lzma2Dec.h
        src/lib/zip/extlibs/lzma/unix/Lzma2Enc.c
        src/lib/zip/extlibs/lzma/unix/Lzma2Enc.h
        src/lib/zip/extlibs/lzma/unix/LzmaDec.c
        src/lib/zip/extlibs/lzma/unix/LzmaDec.h
        src/lib/zip/extlibs/lzma/unix/LzmaEnc.c
        src/lib/zip/extlibs/lzma/unix/LzmaEnc.h
        src/lib/zip/extlibs/lzma/unix/LzmaUtil/Lzma86Dec.c
        src/lib/zip/extlibs/lzma/unix/LzmaUtil/Lzma86Dec.h
        src/lib/zip/extlibs/lzma/unix/LzmaUtil/Lzma86Enc.c
        src/lib/zip/extlibs/lzma/unix/LzmaUtil/Lzma86Enc.h
        src/lib/zip/extlibs/lzma/unix/MtCoder.c
        src/lib/zip/extlibs/lzma/unix/MtCoder.h
        src/lib/zip/extlibs/lzma/unix/Ppmd.h
        src/lib/zip/extlibs/lzma/unix/Ppmd7.c
        src/lib/zip/extlibs/lzma/unix/Ppmd7.h
        src/lib/zip/extlibs/lzma/unix/Ppmd7Dec.c
        src/lib/zip/extlibs/lzma/unix/Ppmd7Enc.c
        src/lib/zip/extlibs/lzma/unix/Ppmd8.c
        src/lib/zip/extlibs/lzma/unix/Ppmd8.h
        src/lib/zip/extlibs/lzma/unix/Ppmd8Dec.c
        src/lib/zip/extlibs/lzma/unix/Ppmd8Enc.c
        src/lib/zip/extlibs/lzma/unix/RotateDefs.h
        src/lib/zip/extlibs/lzma/unix/Sha256.c
        src/lib/zip/extlibs/lzma/unix/Sha256.h
        src/lib/zip/extlibs/lzma/unix/Sort.c
        src/lib/zip/extlibs/lzma/unix/Sort.h
        src/lib/zip/extlibs/lzma/unix/Threads.c
        src/lib/zip/extlibs/lzma/unix/Threads.h
        src/lib/zip/extlibs/lzma/unix/Types.h
        src/lib/zip/extlibs/lzma/unix/Xz.c
        src/lib/zip/extlibs/lzma/unix/Xz.h
        src/lib/zip/extlibs/lzma/unix/XzCrc64.c
        src/lib/zip/extlibs/lzma/unix/XzCrc64.h
        src/lib/zip/extlibs/lzma/unix/XzDec.c
        src/lib/zip/extlibs/lzma/unix/XzEnc.c
        src/lib/zip/extlibs/lzma/unix/XzEnc.h
        src/lib/zip/extlibs/lzma/unix/XzIn.c
        src/lib/zip/extlibs/zlib/adler32.c
        src/lib/zip/extlibs/zlib/compress.c
        src/lib/zip/extlibs/zlib/crc32.c
        src/lib/zip/extlibs/zlib/crc32.h
        src/lib/zip/extlibs/zlib/deflate.c
        src/lib/zip/extlibs/zlib/deflate.h
        src/lib/zip/extlibs/zlib/gzguts.h
        src/lib/zip/extlibs/zlib/infback.c
        src/lib/zip/extlibs/zlib/inffast.c
        src/lib/zip/extlibs/zlib/inffast.h
        src/lib/zip/extlibs/zlib/inffixed.h
        src/lib/zip/extlibs/zlib/inflate.c
        src/lib/zip/extlibs/zlib/inflate.h
        src/lib/zip/extlibs/zlib/inftrees.c
        src/lib/zip/extlibs/zlib/inftrees.h
        src/lib/zip/extlibs/zlib/trees.c
        src/lib/zip/extlibs/zlib/trees.h
        src/lib/zip/extlibs/zlib/uncompr.c
        src/lib/zip/extlibs/zlib/zconf.h
        src/lib/zip/extlibs/zlib/zlib.h
        src/lib/zip/extlibs/zlib/zutil.c
        src/lib/zip/extlibs/zlib/zutil.h
        src/lib/zip/methods/Bzip2Method.h
        src/lib/zip/methods/DeflateMethod.h
        src/lib/zip/methods/ICompressionMethod.h
        src/lib/zip/methods/LzmaMethod.h
        src/lib/zip/methods/StoreMethod.h
        src/lib/zip/methods/ZipMethodResolver.h
        src/lib/zip/streams/compression_decoder_stream.h
        src/lib/zip/streams/compression_encoder_stream.h
        src/lib/zip/streams/crc32stream.h
        src/lib/zip/streams/memstream.h
        src/lib/zip/streams/nullstream.h
        src/lib/zip/streams/serialization.h
        src/lib/zip/streams/streambuffs/compression_decoder_streambuf.h
        src/lib/zip/streams/streambuffs/compression_encoder_streambuf.h
        src/lib/zip/streams/streambuffs/crc32_streambuf.h
        src/lib/zip/streams/streambuffs/mem_streambuf.h
        src/lib/zip/streams/streambuffs/null_streambuf.h
        src/lib/zip/streams/streambuffs/sub_streambuf.h
        src/lib/zip/streams/streambuffs/tee_streambuff.h
        src/lib/zip/streams/streambuffs/zip_crypto_streambuf.h
        src/lib/zip/streams/substream.h
        src/lib/zip/streams/teestream.h
        src/lib/zip/streams/zip_cryptostream.h
        src/lib/zip/utils/enum_utils.h
        src/lib/zip/utils/stream_utils.h
        src/lib/zip/utils/time_utils.h
        src/lib/zip/ZipArchive.cpp
        src/lib/zip/ZipArchive.h
        src/lib/zip/ZipArchiveEntry.cpp
        src/lib/zip/ZipArchiveEntry.h
        src/lib/zip/ZipFile.cpp
        src/lib/zip/ZipFile.h
        src/lib/zip/utils/time_utils.cpp)

set(LIB_FILES
        ${LIB_FS_FILES}
        ${LIB_JSON_FILES}
        ${LIB_ZIP_FILES}
        )

set(MAIN_FILES
        src/main/core/main.cpp
        src/main/core/main.h
        src/main/util/cast/iterators.h src/main/util/numbers.h src/main/util/strings.cpp src/main/util/strings.h src/main/util/MappedIterator.h)

set(SOURCE_FILES
        ${MAIN_FILES}
        ${LIB_FILES}
        )

set(TEST_FILES
        src/test/test.cpp
        src/test/test.h
        src/test/Test.cpp
        src/test/Test.h
        src/test/Tests.cpp
        src/test/Tests.h
        )

add_library(SiliconScratch STATIC ${SOURCE_FILES})

add_executable(SiliconScratch.test ${TEST_FILES})

target_link_libraries(SiliconScratch)
