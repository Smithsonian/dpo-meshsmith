rmdir /S /Q build
mkdir build
cd build

cmake -G "Visual Studio 16 2019" ^
-DCMAKE_BUILD_TYPE=Release ^
..
