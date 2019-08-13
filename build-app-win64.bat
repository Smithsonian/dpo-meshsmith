@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

rmdir /S /Q build
mkdir build
cd build

cmake -G "Visual Studio 16 2019" ^
-DCMAKE_BUILD_TYPE=Release ^
..

msbuild.exe MeshSmith.sln /p:Configuration=Debug
msbuild.exe MeshSmith.sln /p:Configuration=Release
