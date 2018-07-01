Assimp Fix
==========

In order to allow reading large OBJ files, the file ObjFileImporter.cpp needs to be fixed as follows:

```cpp
// Copy vertices of this mesh instance
pMesh->mNumVertices = numIndices;
if (pMesh->mNumVertices == 0) {
    delete pMesh;
    pMesh = nullptr;
    throw DeadlyImportError( "OBJ: no vertices" );
} else if (pMesh->mNumVertices > AI_MAX_VERTICES) {
    delete pMesh;
    pMesh = nullptr;
    throw DeadlyImportError( "OBJ: Too many vertices" );
}
```

See also:
https://github.com/aavenel/assimp/commit/5fba262b2184bd9a50adc64c09a67ede20a6a6d8
