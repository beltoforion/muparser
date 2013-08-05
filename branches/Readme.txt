This directory contains experimental Versions of muParser. 
These versions are unofficial and will not be supported.

muParserSSE 
-----------
A version of muParser using a just in time compiler (asmjit).
This version is approximately twice as fast as the original.

- float datatype only
- only tested on 32 bit systems
- does not have strings and functions with unlimited number of arguments
- does not have xor, or, and and assign operators
- 2-10 times faster than muParser (depending on the expression)

muParserX 
---------
This is a version with multiple datatypes and vector support.

- support for complex numbers, strings, vectors

muParser_cp
-----------
- The muparser archive used for CodeProject. This is the same code as 
  the sourceforge release but with C# wrapper and Visual Studio project 
  files.