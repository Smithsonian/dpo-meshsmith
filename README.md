# MeshSmith
Command line tool for mesh format conversion based on the Assimp mesh library and the Google Draco mesh compression library.

MeshSmith is open source software and part of the Smithsonian 3D Foundation Project. The tool is integrated in [Cook](https://github.com/Smithsonian/dpo-cook), Smithsonian's 3D Processing Pipeline.

## Features
* Converts from/to all available Assimp formats (OBJ, FBX, PLY, Collada, etc.)
* Exports compressed glTF and glb files (with format `gltfx` and `glbx`)
* Simple mesh operations such as coordinate swizzling, scaling, translation
* Inspection feature generates mesh statistics in JSON format

## Author
- [Ralph Wiedemeier, Frame Factory GmbH](https://github.com/framelab)

## Usage
### Command Line Options
```
MeshSmith.exe [OPTION...]

-c, --config arg          JSON configuration file
-i, --input arg           Input file name
-o, --output arg          Output file name
-f, --format arg          Output file format

-j, --joinvertices        Join identical vertices
-n, --stripnormals        Strip normals
-u, --striptexcoords      Strip texture coords
-z, --swizzle arg         Swizzle coordinates
-s, --scale arg           Scale scene by given factor
    --flipuv              Flip UV y coordinate

-a, --diffusemap arg      Diffuse map to be included (gltfx/glbx only)
-b, --occlusionmap arg    Occlusion map to be included (gltfx/glbx only)
-m, --normalmap arg       Normal map to be included (gltfx/glbx only)
-e, --embedmaps           Embed maps (gltfx/glbx only)
-t, --objectspacenormals  Use object space normals (gltfx/glbx only)
-p, --compress            Compress mesh data using Draco (gltfx/glbx only)

-r, --report              Print JSON-formatted report
-l, --list                Print JSON-formatted list of export formats
-v, --verbose             Print log messages to std out
-h, --help                Displays this message
```

### Sample JSON Configuration file
Instead of specifying all parameters on the command line, a JSON configuration file can be specified via `--config` parameter.

If both a command line option and a configuration file with the same option are present, the command line option takes precedence. Note that a few advanced options are only available via configuration file.

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
    "alignX": 0, // 0 = center, -1 = minimum, 1 = maximum
    "alignY": -1,
    "alignZ": 1,
    "flipUV": false,
    "matrix": [ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 ],
    
    "gltfx": {
        "metallicFactor": 0.1,
        "roughnessFactor": 0.8,
        "diffuseMap": "diffuse.jpg",
        "occlusionMap": "occlusion.jpg",
        "emissiveMap": "emissive.jpg",
        "metallicRoughnessMap": "metallic-roughness.jpg",
        "normalMap": "normals.jpg",
        "zoneMap": "zones.jpg",
  
        "objectSpaceNormals":  true,
        "embedMaps": false,
        "useCompression": true
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

### Examples

##### Print all available input and output formats (JSON-formatted)
````
MeshSmith.exe --list
````
##### Call with configuration file
```
MeshSmith.exe --config my_config.json
```
##### Convert mesh from OBJ to PLY format
```
MeshSmith.exe -i input.obj -o output.ply -f plyb
```
##### Scale mesh by factor 0.1
```
MeshSmith.exe -i input.obj -o output.obj -f objnomtl -s 0.1
```
##### Create glTF file (uncompressed)
```
MeshSmith.exe -i mesh.obj -a diffuse.jpg -f gltfx
```
##### Create compressed GLB file with embedded maps
```
MeshSmith.exe -i mesh.obj -a diffuse.jpg -b occlusion.jpg -m normals.jpg --compress --embedmaps -f glbx
```

## Build and Install (Windows)

*Building MeshSmith on your own requires basic knowledge about building and maintaining C++ projects using CMake and Visual Studio. Currently the only supported operating system is Windows.*

### Prerequisites
- [Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/vs/)
- [CMake 3.13.0 or higher](https://cmake.org/)

### 3rd Pary Libraries
#### Assimp

Assimp is a library to import and export various 3d-model-formats including scene-post-processing to generate missing render data.

MeshSmith uses a modified version of Assimp containing a number of fixes for handling large OBJ files.

- Modified version: https://github.com/framelab/assimp
- Original repository: https://github.com/assimp/assimp

#### Draco

Draco is a library for compressing and decompressing 3D geometric meshes and point clouds. It is intended to improve the storage and transmission of 3D graphics.

- Repository: https://github.com/google/draco

#### Building the Libraries

Assimp and Draco are included in the MeshSmith repository as submodules. They can be found
in the `libs` subfolder. In order to properly clone the submodules, you need to clone
MeshSmith using the `--recurse-submodules` option.

```
git clone --recurse-submodules https://github.com/Smithsonian/dpo-meshsmith
```

For convenience, a build script is provided in the MeshSmith root folder. It requires Visual Studio 2019 to be installed in its default location. Open a command
prompt and enter the following:

```
.\build-libs-win64.bat
```

#### Building the  MeshSmith CLI Application

After building the libraries, the application build (debug and release versions) can be started like so:
```
.\build-app-win64.bat
```

##### Build output

- Debug version: `bin\debug`
- Release version: `bin\release`
