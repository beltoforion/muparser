Echo Cleaning up

ECHO This will erase all pdb, obj, ilk, idb, pch, ncb, exp files
pause

REM Delete all unnecessary files
del *.bak /s
del *.sbr /s
del *.pdb /s
del *.obj /s
del *.ilk /s
del *.idb /s
del *.pch /s
del *.ncb /s
del *.exp /s
del *.log /s
del *.manifest /s
del *.dep /s
del *.user /s
del /aH *.suo /s
rd ParserLib\Debug /s /q
rd ParserLib\Release /s /q
rd ParserDLL\Debug /s /q
rd ParserDLL\Release /s /q

; Clear ParserLib
cd ParserLib
del *.lib
cd ..

; Clear example1
cd example1
del example1.exe
rd Debug /s /q
rd Release /s /q
cd ..

; Clear example2
cd example2
del example2.exe
rd Debug /s /q
rd Release /s /q
cd ..

; Clear example_csharp
cd example_csharp
rd bin /s /q
rd obj /s /q
cd ..
ECHO Done; press any key
pause
