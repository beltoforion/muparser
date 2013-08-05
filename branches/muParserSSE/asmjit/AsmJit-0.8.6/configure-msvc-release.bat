mkdir Build
cd Build
cmake .. -DCMAKE_BUILD_TYPE=Release -DASMJIT_BUILD_LIBRARY=1 -DASMJIT_BUILD_TEST=1 -G"Visual Studio 8 2005"
cd ..
