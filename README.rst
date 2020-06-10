.. image:: https://travis-ci.org/beltoforion/muparser.svg?branch=master
    :target: https://travis-ci.org/beltoforion/muparser

.. image:: https://ci.appveyor.com/api/projects/status/u4882uj8btuspj9x?svg=true
    :target: https://ci.appveyor.com/project/jschueller/muparser-9ib44


muparser - fast math parser library
===================================

For a detailed description of the parser go to http://beltoforion.de/article.php?a=muparser.

See Install.txt for installation

Change Notes for Revision 2.2.7 (in Development;10.06.2020)
===========================================================

Changes:
--------
* using OpenMP is now the default settings for cmake based builds
* added optimization for trivial expressions. (Expressions with an RPN length of 1)
* introduced a maximum length for expressions (5000 Character)
* introduced a maximum length for identifiers (100 Characters)
* removed the MUP_MATH_EXCEPTION macro and related functionality. (C++ exceptions for divide by zero or sqrt of a negative number are no longer supported)
* removed ParserStack.h (replaced with std::stack)
* removed macros for defining E and PI (replaced with a static class)
* source code is now aimed at C++17
* the MUP_ASSERT macro is no longer removed in release builds for better protection against segmentation faults

Security Fixes: (The issues were present in all prior stable releases)
----------------------------------------------------------------------

* Prevented multiple access violations for malformed expressions with if then else and functions taking multiple arguments like "sum(0?1,2,3,4:5)"
* Added additional runtime checks for release builds to prevent segmentation faults for invalid expressions

Bugfixes:
---------
* Fixed an issue where the bulk mode could hang on GCC/CLANG builds due to OpenMP chunksize dropping below 1.

