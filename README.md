[![GitHub issues](https://img.shields.io/github/issues/beltoforion/muparser.svg?maxAge=360)](https://github.com/beltoforion/muparser/issues)
[![Version](https://img.shields.io/github/release/beltoforion/muparser.svg?maxAge=360)](https://github.com/beltoforion/muparser/blob/master/CHANGELOG)
[![Packaging status](https://repology.org/badge/tiny-repos/muparser.svg)](https://repology.org/project/muparser/versions)

muparser - fast math parser library
===================================

![title](https://github.com/beltoforion/muparser/assets/2202567/fbeb2347-9884-4dd7-a3c9-112b605d7390)

Change Notes for Revision 2.3.6 (2026-11-10)
===========================
  
  Changes:
  -----------
  - Issue https://github.com/beltoforion/muparser/issues/161: Add HasFun function added to query the existance of a function under a given name.
  - Issue https://github.com/beltoforion/muparser/issues/93: Add cotangent function
  - Issue https://github.com/beltoforion/muparser/issues/158: Add callback type multstrfun_type for functions taking a string constant followed by a variable number of numeric arguments (at least one), i.e. printf("x={0}, y={1}", a, b).

  Security Fixes:
  -----------
  - Issue https://github.com/beltoforion/muparser/issues/167: Add additional checks to eliminate possibility of access violations due to malformed expressions. I dont have test cases and do not know whether this issue was exploitable at all but fixing them made sense.
  - Issue https://github.com/beltoforion/muparser/issues/168: ClearConst() now also clears the internal string variable buffer (m_vStringVarBuf) to keep it in sync with the name-to-index map (m_StrVarDef). Previously stale entries accumulated across repeated ClearConst() calls.
  - Issue https://github.com/beltoforion/muparser/pull/170: Several memory safety, undefined behaviour and thread safety fixes, among them a use-after-free in mupRelease, exceptions crossing the C API boundary and data races on static buffers of the C API.
  - Issue https://github.com/beltoforion/muparser/issues/171: ParserInt now throws ecDOMAIN_ERROR instead of causing a signed integer overflow in Add, Sub, Mul, the unary minus and when rounding values that do not fit into an int.
  - Issue https://github.com/beltoforion/muparser/issues/176: ParserInt::IsBinVal performed the bit shift for binary literals in a signed type, triggering sanitizer reports for literals starting with 1.

  Bug Fixes:
  -----------
  - Issue https://github.com/beltoforion/muparser/issues/165: String constants used as function arguments were pushed twice into the internal string buffer.
  - Issue https://github.com/beltoforion/muparser/issues/173: Constant folding of "&&" and "||" compares the operands with zero instead of casting them to int. Results for fractional and out-of-range values now match the evaluation without optimizer.
  - Issue https://github.com/beltoforion/muparser/pull/175: A division by a constant zero was dropped from the bytecode when the optimizer was active (i.e. "asin(3/0)" returned 3). It is now evaluated at run time.

  Fixed Compiler Warnings and Errors:
  -----------
  - Issue https://github.com/beltoforion/muparser/issues/160: Refacturing for better source code compatibility with Fil-C; needlessly "clever" code replaced with a more maintainable version.
    
Change Notes for Revision 2.3.5
===========================

New Features:
-----------
- added a rnd() function
  
Fixed Compiler Warnings:
-----------
- Fix problem with IntelLLVM fast math

Build System:  
------------
- fix for https://github.com/beltoforion/muparser/issues/127 minimum required cmake version set to 3.15
- fix for https://github.com/beltoforion/muparser/issues/132 example1 fails to builds on Windows with mingw gcc
- fix for https://github.com/beltoforion/muparser/issues/147 Build failed with MSVC/C++20

Change Notes for Revision 2.3.4  
===========================

Maintainance Release with updates of the cmake build system.

Build System:  
------------
- cmake is using OpenMP target and setting _UNICODE preprocessor definition

Fixed Compiler Warnings:
-----------
- fix for https://github.com/beltoforion/muparser/issues/117 (sprintf deprecated)

Change Notes for Revision 2.3.3  
===========================
To read the full documentation please go to: http://beltoforion.de/en/muparser.

See Install.txt for installation

Security Fixes:  
------------
The following new issues, discovered by oss-fuzz are fixed: 

* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=24167 (Abrt)
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=24355 (Heap-buffer-overflow READ 8)
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=25402 (Heap-buffer-overflow READ 8)

Bugfixes:
-----------
* Fixed a couple of issues for building the C-Interface (muParserDLL.cpp/.h) with wide character support.
* fix for https://github.com/beltoforion/muparser/issues/93
* fix for https://github.com/beltoforion/muparser/issues/94
* fix for https://github.com/beltoforion/muparser/issues/110; new expression size limit is 20000

Fixed Compiler Warnings:
-----------
* Visual Studio: Disabled compiler warning 26812 (Prefer 'enum class' over 'enum') Use of plain old enums has not been deprecated and only MSVC is complaining. 
* Visual Studio: Disabled compiler warning 4251 (... needs to have dll-interface to be used by clients of class ...)  For technical reason the DLL contains the class API and the DLL API. Just do not use the class API if you intent to share the dll accross windows versions. (The same is true for Linux but distributions do compile each application against their own library version anyway)

Changes:
------------
* Adding manual definitions to avoid potential issues with MSVC
* Adding missing overrides
* Added a new option "-DENABLE_WIDE_CHAR" to CMake for building muparser with wide character support
* export muparser targets, such that client projects can import it using find_package() (https://github.com/beltoforion/muparser/pull/81#event-3528671228)




