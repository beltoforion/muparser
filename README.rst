.. image:: https://travis-ci.org/beltoforion/muparser.svg?branch=master
    :target: https://travis-ci.org/beltoforion/muparser

.. image:: https://ci.appveyor.com/api/projects/status/u4882uj8btuspj9x?svg=true
    :target: https://ci.appveyor.com/project/beltoforion/muparser

.. image:: https://img.shields.io/github/issues/beltoforion/muparser.svg?maxAge=360
    :target: https://github.com/beltoforion/muparser/issues
 
.. image:: https://img.shields.io/github/release/beltoforion/muparser.svg?maxAge=360
    :target: https://github.com/beltoforion/muparser/blob/master/CHANGELOG
 
.. image:: https://repology.org/badge/tiny-repos/muparser.svg
    :target: https://repology.org/project/muparser/versions


muparser - Fast Math Parser 2.3.2 
===========================
.. image:: http://beltoforion.de/en/muparser/images/title.jpg


To read the full documentation please go to: http://beltoforion.de/en/muparser.

See Install.txt for installation

Change Notes for Revision 2.3.3  (Development)
===========================
Security Fixes:  
------------
The following issue was newly discovered and is present in previous releases.

* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=24167 (Abrt)

Bugfixes:
-----------
* fixed a couple of issues for building the C-Interface (muParserDLL.cpp/.h) with wide character support.

Fixed Compiler Warnings:
-----------
* Visual Studio: Disabled compiler warning 26812 (Prefer 'enum class' over 'enum') Use of plain old enums has not been deprecated and only MSVC is complaining. 
* Visual Studio: Disabled compiler warning 4251 (... needs to have dll-interface to be used by clients of class ...)  For technical reason the DLL contains the class API and the DLL API. Just do not use the class API if you intent to share the dll accross windows versions. (The same is true for Linux but distributions do compile each application against their own library version anyway)

Changes:
------------
* Added a new option "-DENABLE_WIDE_CHAR" to CMake for building muparser with wide character support
* export muparser targets, such that client projects can import it using find_package() (https://github.com/beltoforion/muparser/pull/81#event-3528671228)

Change Notes for Revision 2.3.2
===========================
Changes:
------------
* removed "final" keyword from Parser class since this API change broke multiple client applications

Security Fixes:  
------------
The following issue was present in all older releases.

* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=23410 (Heap-buffer-overflow)

API and ABI compliance check with version 2.2.6
------------

Version 2.3 will extend existing enumerators. New Error codes have been added. In the muparser base class protected functions for implementing basic mathematical operations such as sin,cos, sqrt,tan,... have been removed.

The binary interface should be compatible with versions 2.2.6 unless the parser is used in ways that i did not forsee. I checked the compliance against the sample application compiled for 2.2.6 by exchanging the library with the new version 2.3. I did not see any problems. You can find a complete ABI compliance report here:

https://www.beltoforion.de/en/muparser/compat_reports/2.2.6_to_2.3.2/compat_report.html

I recommend replacing existing versions of 2.2.6 with version 2.3.2. Please report all incompatibilities that you find (API and ABI). I will try to fix them (if reasonable)


Change Notes for Revision 2.3.1
===========================
No changes, only prereleases exist. Version 2.3.2 replaced them.


Change Notes for Revision 2.3.0
===========================

Version 2.3.0 will bring fixes for parsing in bulk mode. It will enable OpenMP by default thus allowing the parallelization of expression evaluation. It will also fix a range of issues reported by oss-fuz (https://github.com/google/oss-fuzz).

Changes:
------------

* using OpenMP is now the default settings for cmake based builds
* added optimization for trivial expressions. (Expressions with an RPN length of 1)
* introduced a maximum length for expressions (5000 Character)
* introduced a maximum length for identifiers (100 Characters)
* removed the MUP_MATH_EXCEPTION macro and related functionality. (C++ exceptions for divide by zero or sqrt of a negative number are no longer supported)
* removed ParserStack.h (replaced with std::stack)
* removed macros for defining E and PI 
* the MUP_ASSERT macro is no longer removed in release builds for better protection against segmentation faults

Bugfixes:
------------
Fixed several issues reported by oss-fuzz. The issues were present in older releases. Most of them resulted in segmentation faults.

* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=23330
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=22922
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=22938
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=23330
* Added additional runtime checks for release builds to prevent segmentation faults for invalid expressions
* Fixed an issue where the bulk mode could hang on GCC/CLANG builds due to OpenMP chunksize dropping below 1.

