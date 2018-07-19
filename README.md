# MeshSmith
Mesh conversion tool based on the Assimp mesh library and the Google Draco mesh compression library.

## Features
* Converts from/to all available Assimp formats (OBJ, FBX, PLY, Collada, etc.)
* Exports compressed glTF and glb files (with format `gltfx` and `glbx`)
* Simple mesh operations such as coordinate swizzling, scaling, translation
* Reports mesh statistics in JSON format

## Usage
```
Usage:
  MeshSmith CLI v0.92 [OPTION...]

  -c, --config arg          JSON configuration file
  -i, --input arg           Input file name
  -o, --output arg          Output file name
  -f, --format arg          Output file format
  -a, --diffusemap arg      Diffuse map file (gltfx/glbx only)
  -b, --occlusionmap arg    Occlusion map file (gltfx/glbx only)
  -m, --normalmap arg       Normal map file (gltfx/glbx only)
  -e, --embedmaps           Embed map images (gltfx/glbx only)
  -t, --objectspacenormals  Use object space normals (gltfx/glbx only)
  -p, --compress            Compress mesh data using Draco (gltfx/glbx only)
  -j, --joinvertices        Join identical vertices
  -n, --stripnormals        Strip normals
  -u, --striptexcoords      Strip texture coords
  -z, --swizzle arg         Swizzle coordinates
  -s, --scale arg           Scale scene by given factor
  -r, --report              Print JSON-formatted report
  -l, --list                Print JSON-formatted list of export formats
  -v, --verbose             Print log messages to std out
  -h, --help                Displays this message
```
#### Sample JSON Configuration file
Instead of specifying all parameters on the command line, a JSON configuration file can be used.
```json
{
  "input": "input-mesh.obj",
  "output": "output.glb",
  "format": "glbx",
  "verbose": false,
  "report": false,
  "list": false,
  "joinVertices": false,
  "stripNormals": false,
  "stripTexCoords": false,
  "swizzle": "X+Y+Z+",
  "scale": 1.0,
  "translate": [ 0.0, 0.0, 0.0 ],
  "gltfx": {
    "useCompression": true,
    "diffuseMap": "diffuse.jpg",
    "occlusionMap": "occlusion.jpg",
    "normalMap": "normals.jpg",
    "objectSpaceNormals":  true,
    "embedMaps": false
  },
  "compression": {
    "positionQuantizationBits": 14,
    "texCoordsQuantizationBits": 12,
    "normalsQuantizationBits": 10,
    "genericQuantizationBits": 8,
    "compressionLevel": 8
  }
}
```

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
* Open Visual Studio 2017 project: build/Assimp.sln
* Build > Batch Build > Select `Assimp Debug` and `Release` builds
* Copy `build/code/Debug` and `build/code/Release` to `vendor/assimp/lib`
* Copy `include` and `build/include` to `vendor/assimp/include`

#### Google Draco

* Clone from https://github.com/google/draco
* Build with cmake in separate `build` folder as follows

```
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
* Open Visual Studio 2017 project: `build/draco.sln`
* *Build > Batch Build*, select `Draco Debug/Release` and `INSTALL Debug/Release` builds
* Copy `build/Debug` and `build/Release` to `vendor/draco/lib`
* Copy `C:\Program Files\draco\include` and `build/include` to `vendor/draco/include`

### Build Meshsmith Application

* Open project in Visual Studio: *File > Open > CMake...*
* Select *CMake > Change CMake Settings...*
  * Remove x86 Debug and x86 Release configurations
  * Change type of x64 Release configuration to "Release"
* *CMake > Build*
* Copy Assimp DLLs to `bin/Debug` and `bin/Release` respectively