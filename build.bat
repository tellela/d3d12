@echo off

cl /nologo /Zi /W3 hello.cpp

doskey clean=del *.exe *.obj *.pdb *.ilk

doskey run=hello.exe
