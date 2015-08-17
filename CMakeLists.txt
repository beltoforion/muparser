
cmake_minimum_required (VERSION 2.8)

if (POLICY CMP0018)
  cmake_policy (SET CMP0018 OLD)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project (muparser)

option (BUILD_SHARED_LIBS "build shared/static libs" ON)
option (BUILD_SAMPLES "build samples" ON)
option (USE_OPENMP "Use OpenMP for multithreading" OFF)

set (MUPARSER_INCLUDE_DIR include CACHE STRING "include path")
set (MUPARSER_LIB_DIR lib${LIB_SUFFIX} CACHE STRING "lib path")
set (CONFIG_CMAKE_PATH ${MUPARSER_LIB_DIR}/cmake/muparser CACHE STRING "cmake config path") 

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions (-D_DEBUG)
endif ()

if (USE_OPENMP)
  find_package (OpenMP)
  if (OPENMP_FOUND)
    set (MUP_USE_OPENMP TRUE)
    set (HAVE_OMP_H TRUE)
    set (CMAKE_CXX_FLAGS "${OpenMP_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
    set (CMAKE_SHARED_LIBRARY_CXX_FLAGS "${OpenMP_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
  endif ()
endif ()

if (MSVC)
  # Disable some warnings
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4996")
endif ()

# TODO: 
# create a muParserDef.h.in to configure muParserDef.h according to build options
# muParserDef.h.in: #cmakedefine MUP_USE_OPENMP 
# configure_file (include/muParserDef.h.in ${CMAKE_BINARY_DIR}/include/muParserDef.h)
# include_directories (${CMAKE_BINARY_DIR}/include)

configure_file (include/muParserDef.h.in muParserDef.h)

file (GLOB headers "include/*.h")
list (APPEND headers ${CMAKE_CURRENT_BINARY_DIR}/muParserDef.h)
file (GLOB sources "src/*.cpp")
include_directories (include)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

add_library (muparser ${sources})
if (BUILD_SHARED_LIBS)
  set_target_properties (muparser PROPERTIES COMPILE_DEFINITIONS "MUPARSERLIB_EXPORTS")
else ()
  # clients of the static library must define MUPARSER_STATIC
  set (MUPARSER_DEFINITIONS "-DMUPARSER_STATIC")
endif ()

add_definitions (${MUPARSER_DEFINITIONS})

# Add targets to the build-tree export set
export (TARGETS muparser FILE "${PROJECT_BINARY_DIR}/muParser-Targets.cmake")

# Install the export set for use with the install-tree
install (EXPORT muParser-Targets DESTINATION
  "${CONFIG_CMAKE_PATH}")

install (TARGETS muparser
         EXPORT muParser-Targets
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION ${MUPARSER_LIB_DIR}
         ARCHIVE DESTINATION ${MUPARSER_LIB_DIR})

install (FILES ${headers} DESTINATION include)

if (BUILD_SAMPLES)
  add_executable (sample1 samples/example1/example1.cpp)
  target_link_libraries (sample1 muparser)

  add_executable (sample2 samples/example2/example2.c)
  target_link_libraries (sample2 muparser)
endif ()

# build tarballs via package_source target
set (CPACK_PACKAGE_VERSION_MAJOR 2)
set (CPACK_PACKAGE_VERSION_MINOR 2)
set (CPACK_PACKAGE_VERSION_PATCH 5)
set (CPACK_SOURCE_GENERATOR      "ZIP;TGZ")
set (CPACK_SOURCE_IGNORE_FILES "/.git;/build;.*~;${CPACK_SOURCE_IGNORE_FILES}" )
set (PACKAGE_NAME ${PROJECT_NAME})
set (PACKAGE_VERSION        ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
include (CPack)




# create muparser.pc
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix "\${prefix}")
set (libdir "\${exec_prefix}/lib")
set (includedir "\${prefix}/include")
configure_file (cmake/muparser.pc.in muparser.pc @ONLY)
install (FILES ${CMAKE_BINARY_DIR}/muparser.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)


configure_file (cmake/muParserConfig.cmake.in muParserConfig.cmake @ONLY)
configure_file (cmake/muParserConfigVersion.cmake.in muParserConfigVersion.cmake @ONLY)


install (FILES ${CMAKE_CURRENT_BINARY_DIR}/muParserConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/muParserConfigVersion.cmake
         DESTINATION "${CONFIG_CMAKE_PATH}")


