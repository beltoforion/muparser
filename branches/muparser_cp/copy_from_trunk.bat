Echo Copying source code from trunk

cd ParserLib

copy ..\..\..\trunk\include\*.h .
copy ..\..\..\trunk\src\*.cpp .

cd ..
copy ..\..\trunk\samples\example1\example1.cpp example1\example1.cpp
copy ..\..\trunk\samples\example2\example2.c example2\example2.c

rem Die DLL Dateien in eigenen Ordner verschieben
move muParserDLL.h ..\ParserDLL
move muParserDLL.cpp ..\ParserDLL

ECHO Done; press any key
pause
