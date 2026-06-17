#!/bin/bash

echo ""
echo "Generating build files with Premake..."
cd build
premake5 gmake
cd ..

echo ""
echo "Building the project..."
make config=release
echo ""
echo "Done! The executable file should be in the ./bin/Release/ directory."
make clean