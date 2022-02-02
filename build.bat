@echo off

cl /nologo /Zi /W3 hello.cxx

doskey clean=del *.exe *.obj *.pdb *.ilk

doskey run=hello.exe
