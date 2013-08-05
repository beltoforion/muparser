# Reguire minimum version of CMake
CMake_Minimum_Required(VERSION 2.6)

# AsmJit project - Need to use both C and C++
Project(AsmJit C CXX)

# If ASMJIT_DIR is not specified, assume that we are building it from here
If(NOT ASMJIT_DIR)
  Set(ASMJIT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
EndIf(NOT ASMJIT_DIR)

# AsmJit C++ sources
Set(ASMJIT_SOURCES
  AsmJit/AssemblerX86X64.cpp
  AsmJit/CompilerX86X64.cpp
  AsmJit/CpuInfo.cpp
  AsmJit/Defs.cpp
  AsmJit/Lock.cpp
  AsmJit/Logger.cpp
  AsmJit/LoggerX86X64.cpp
  AsmJit/LoggerX86X64Dump.cpp
  AsmJit/MemoryManager.cpp
  AsmJit/SerializerX86X64.cpp
  AsmJit/Util.cpp
  AsmJit/VirtualMemory.cpp
)

# AsmJit C++ headers
Set(ASMJIT_HEADERS
  AsmJit/AsmJit.h
  AsmJit/Assembler.h
  AsmJit/AssemblerX86X64.h
  AsmJit/Build.h
  AsmJit/Compiler.h
  AsmJit/CompilerX86X64.h
  AsmJit/Config.h
  AsmJit/CpuInfo.h
  AsmJit/Defs.h
  AsmJit/DefsX86X64.h
  AsmJit/Lock.h
  AsmJit/Logger.h
  AsmJit/LoggerX86X64.h
  AsmJit/MemoryManager.h
  AsmJit/Serializer.h
  AsmJit/SerializerX86X64.h
  AsmJit/Util.h
  AsmJit/VirtualMemory.h
  AsmJit/WarningsPop.h
  AsmJit/WarningsPush.h
)

# Include AsmJit to be able to use #include <AsmJit/...>
Include_Directories(${ASMJIT_DIR})

# pthread library is needed for non-windows OSes.
If(NOT WIN32)
  Link_Libraries(pthread)
EndIf(NOT WIN32)

# Debugging support is indicated through _DEBUG macro, add it if needed
If(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  Add_Definitions(-D_DEBUG)
EndIf(${CMAKE_BUILD_TYPE} MATCHES "Debug")

# Build AsmJit shared library?
If(ASMJIT_BUILD_LIBRARY)
  Add_Library(AsmJit SHARED ${ASMJIT_SOURCES} ${ASMJIT_HEADERS})
  Install(TARGETS AsmJit DESTINATION lib)

  # Install header files.
  ForEach(i ${ASMJIT_HEADERS})
    Get_Filename_Component(path ${i} PATH)
    Install(FILES ${i} DESTINATION "include/${path}")
  EndForEach(i)
EndIf(ASMJIT_BUILD_LIBRARY)

# Build AsmJit test executables?
If(ASMJIT_BUILD_TEST)
  Set(ASMJIT_TEST_FILES
    testcompiler
    testcpu
    testdummy
    testfunclargeargs
    testjit
    testjumpshort
    testjumptable
    testmemorymanager
    testopcode
    testrep
    testrewrite
    testsetcc
    teststate
    testtrampoline
    testvariables
    testvarmemalloc
    testvarspill
  )

  ForEach(file ${ASMJIT_TEST_FILES})
    Add_Executable(${file} test/${file}.cpp)
    Target_Link_Libraries(${file} AsmJit)
  EndForEach(file)
EndIf(ASMJIT_BUILD_TEST)
