@echo off

cd gen >NUL 2>NUL

set CODE=make_mat_h.cpp
set EXECUTABLE=make_mat_h.exe
set OUTPUT=mat.h

g++ %CODE% -o %EXECUTABLE% -std=c++17 -Wall -Wextra -pedantic-errors
if not %ERRORLEVEL% == 0 (
	echo Compilation failed. 
	exit /B 1
)

%EXECUTABLE%
if not %ERRORLEVEL% == 0 (
	del /F /Q %EXECUTABLE%
    echo Generation failed.
	exit /B 1
)

del /F /Q %EXECUTABLE% >NUL 2>NUL

move /Y %OUTPUT% ../src >NUL 2>NUL
if not %ERRORLEVEL% == 0 (
    echo Can't move the file to the target directory.
	exit /B 1
)

g++ ../src/%OUTPUT% -std=c++17 -Wall -Wextra -pedantic-errors -fsyntax-only
if not %ERRORLEVEL% == 0 (
    echo Syntax check failed.
	exit /B 1
)
