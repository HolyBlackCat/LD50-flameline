cd gen >NUL 2>NUL

set CODE=make_mat_h.cpp
set EXECUTABLE=make_mat_h.exe
set OUTPUT=mat.h
set DESTINATION=src/utils

g++ %CODE% -o %EXECUTABLE% -std=c++2a -Wall -Wextra -pedantic-errors -static
@if not %ERRORLEVEL% == 0 (
	echo Compilation failed. 
	@pause
	@exit /B 1
)

%EXECUTABLE%
@if not %ERRORLEVEL% == 0 (
	del /F /Q %EXECUTABLE%
    echo Generation failed.
	@pause
	@exit /B 1
)

del /F /Q %EXECUTABLE% >NUL 2>NUL

move /Y %OUTPUT% ../%DESTINATION% >NUL 2>NUL
@if not %ERRORLEVEL% == 0 (
    echo Can't move the file to the target directory.
	@pause
	@exit /B 1
)

g++ ../%DESTINATION%/%OUTPUT% -std=c++2a -Wall -Wextra -pedantic-errors -fsyntax-only
@if not %ERRORLEVEL% == 0 (
    echo Syntax check failed.
	@pause
	@exit /B 1
)

@color 0a
pause