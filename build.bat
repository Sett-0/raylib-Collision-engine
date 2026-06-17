@echo off 
echo:
echo Generating build files with Premake...
cd build
premake5.exe gmake
if %errorlevel% neq 0 (
    echo Failed to generate Makefile. Make sure premake5 is in your PATH.
    pause
    exit /b %errorlevel%
)
cd ..

echo:
echo Building the project...
make config=release
echo:
echo Done! The executable file should be in the bin/Release/ directory.
make clean