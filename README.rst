.. image:: https://travis-ci.org/beltoforion/muparser.svg?branch=master
    :target: https://travis-ci.org/beltoforion/muparser

.. image:: https://ci.appveyor.com/api/projects/status/u4882uj8btuspj9x?svg=true
    :target: https://ci.appveyor.com/project/jschueller/muparser-9ib44


muparser - fast math parser library
===================================

For a detailed description of the parser go to http://beltoforion.de/article.php?a=muparser.

See Install.txt for installation

Change Notes for Revision 2.3.0 (in Development;10.06.2020)
===========================================================

Version 2.3.0 will bring fixes for parsing in bulk mode. It will enable OpenMP by default thus allowing the parallelization of expression evaluation. It will also fix a range of issues reported by oss-fuz (https://github.com/google/oss-fuzz).

Changes:
--------
* using OpenMP is now the default settings for cmake based builds
* added optimization for trivial expressions. (Expressions with an RPN length of 1)
* introduced a maximum length for expressions (5000 Character)
* introduced a maximum length for identifiers (100 Characters)
* removed the MUP_MATH_EXCEPTION macro and related functionality. (C++ exceptions for divide by zero or sqrt of a negative number are no longer supported)
* removed ParserStack.h (replaced with std::stack)
* removed macros for defining E and PI 
* source code is now aimed at C++17
* the MUP_ASSERT macro is no longer removed in release builds for better protection against segmentation faults

Security Fixes: 
----------------------------------------------------------------------
Fixed several issues reported by oss-fuzz. The issues were present in older releases. Most of them resulted in segmentation faults.

* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=23330
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=22922
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=22938
* https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=23330
* Added additional runtime checks for release builds to prevent segmentation faults for invalid expressions

Bugfixes:
---------
* Fixed an issue where the bulk mode could hang on GCC/CLANG builds due to OpenMP chunksize dropping below 1.

