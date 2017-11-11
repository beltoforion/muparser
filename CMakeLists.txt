# CMake based on work from @xantares
cmake_minimum_required(VERSION 3.1)
project(muParserProject)

include(CTest)
enable_testing()

# Bump versions on release
set(MUPARSER_VERSION_MAJOR 2)
set(MUPARSER_VERSION_MINOR 2)
set(MUPARSER_VERSION_PATCH 5)
set(MUPARSER_VERSION ${MUPARSER_VERSION_MAJOR}.${MUPARSER_VERSION_MINOR}.${MUPARSER_VERSION_PATCH})

# Build options
option(ENABLE_SAMPLES "Build the samples" ON)
option(ENABLE_OPENMP "Enable OpenMP for multithreading" OFF)

if(ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)
    set(CMAKE_CXX_FLAGS "${OpenMP_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "${OpenMP_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(muparser
    src/muParserBase.cpp
    src/muParserBytecode.cpp
    src/muParserCallback.cpp
    src/muParser.cpp
    src/muParserDLL.cpp
    src/muParserError.cpp
    src/muParserInt.cpp
    src/muParserTest.cpp
    src/muParserTokenReader.cpp
)
target_compile_definitions(muparser PRIVATE MUPARSERLIB_EXPORTS)
if(ENABLE_OPENMP)
    target_compile_definitions(muparser PRIVATE MUP_USE_OPENMP)
endif()
set_target_properties(muparser PROPERTIES
    VERSION ${MUPARSER_VERSION}
    SOVERSION ${MUPARSER_VERSION_MAJOR}
)

# Install the export set for use with the install-tree
export(TARGETS muparser FILE "${CMAKE_BINARY_DIR}/muparser-targets.cmake")

if(ENABLE_SAMPLES)
    # the C example does not compile at the moment, because definitions must be
    # inside external "C" { } to avoid name mangling.
    #add_executable(example2 samples/example2/example2.c)
    #target_link_libraries(example2 muparser)
    
    add_executable(example1 samples/example1/example1.cpp)
    target_link_libraries(example1 muparser)
    add_test(base example1)
endif()

install(TARGETS muparser DESTINATION lib)
install(FILES
    include/muParserBase.h
    include/muParserBytecode.h
    include/muParserCallback.h
    include/muParserDef.h
    include/muParserDLL.h
    include/muParserError.h
    include/muParserFixes.h
    include/muParser.h
    include/muParserInt.h
    include/muParserStack.h
    include/muParserTemplateMagic.h
    include/muParserTest.h
    include/muParserToken.h
    include/muParserTokenReader.h
    DESTINATION include
)

# Define variables for the pkg-config file
# The GNUInstallDirs defines ${CMAKE_INSTALL_DATAROOTDIR}
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)
set(PACKAGE_NAME muparser)
configure_file(
    "${CMAKE_SOURCE_DIR}/build/autoconf/muparser.pc.cmakein"
    "${CMAKE_BINARY_DIR}/muparser.pc"
    @ONLY
)
install(
    FILES "${CMAKE_BINARY_DIR}/muparser.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
