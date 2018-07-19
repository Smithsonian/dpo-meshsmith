# MeshSmith
Mesh conversion tool based on the Assimp mesh library.

## Build and Install
### 3rd Pary Libraries
#### Assimp

* Clone from https://github.com/framelab/assimp
* Build with cmake in separate `build` folder as follows

```
mkdir build
cd build

cmake -G "Visual Studio 15 2017 Win64" ..
```
* Open Visual Studio project: build/Assimp.sln
* Build > Batch Build > Select `Debug` and `Release` builds
* Copy `build/code/debug` and `build/code/release` to `vendor/assimp/lib`
* Copy `include` and `build/include` to `vendor/assimp/include`

#### Google Draco

* Clone from https://github.com/google/draco
* Build with 