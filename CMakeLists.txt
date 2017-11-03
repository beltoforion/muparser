cmake_minimum_required(VERSION 3.1)
project(muParserProject)

include_directories(${CMAKE_SOURCE_DIR}/include)
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

add_executable(example1 samples/example1/example1.cpp)
target_link_libraries(example1 muparser)
# the C example does not compile at the moment, because definitions must be
# inside external "C" { } to avoid name mangling.
#add_executable(example2 samples/example2/example2.c)
#target_link_libraries(example2 muparser)

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
