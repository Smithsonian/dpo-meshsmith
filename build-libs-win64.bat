@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

cd libs\assimp

rmdir /S /Q build
mkdir build
cd build

cmake -G "Visual Studio 16 2019" ^
-DCMAKE_BUILD_TYPE=Release ^
..

msbuild.exe Assimp.sln /t:assimp:Rebuild /p:Configuration=Debug
msbuild.exe Assimp.sln /t:assimp:Rebuild /p:Configuration=Release

rmdir /S /Q ..\bin
mkdir ..\bin

set COPYCMD=/Y
xcopy code\Debug\* ..\bin\lib\debug /I
xcopy code\Release\* ..\bin\lib\release /I
xcopy ..\include\* ..\bin\include /S /I
xcopy include\* ..\bin\include /S /I

cd ..\..\..


cd libs\draco

rmdir /S /Q build
mkdir build
cd build

cmake -G "Visual Studio 16 2019" ^
-DCMAKE_BUILD_TYPE=Release ^
..

msbuild.exe draco.sln /t:draco:Rebuild /p:Configuration=Debug
msbuild.exe draco.sln /t:draco:Rebuild /p:Configuration=Release

rmdir /S /Q ..\bin
mkdir ..\bin

set COPYCMD=/Y
xcopy Debug\* ..\bin\lib\debug /I
xcopy Release\* ..\bin\lib\release /I
xcopy ..\src\*.h ..\bin\include /S /I
xcopy draco\draco_features.h ..\bin\include\draco /I

cd ..\..\..
