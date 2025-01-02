#!/bin/bash

# Compile C code into shared library
gcc -shared -fPIC -o simulator/libptcg.so simulator/*.c

# Compile Cython module
python setup.py build_ext --inplace

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed. Please check the error messages above."
fi
