/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "GLTFExporter.h"

#include "gltf/gltf.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>

#include <boost/filesystem.hpp>

#include <limits>
#include <iostream>

using namespace meshsmith;
using namespace flow;
using namespace boost;
using std::string;
using std::cout;
using std::endl;


GLTFExporter::GLTFExporter(const aiScene* pScene) :
	_verbose(false),
	_pScene(pScene),
	_exportNormals(true),
	_exportUVs(true),
	_enableCompression(false)
{
}

GLTFExporter::~GLTFExporter()
{
}

void GLTFExporter::setVerbose(bool state)
{
	_verbose = state;
}

void GLTFExporter::setDiffuseMapFileName(const string& fileName)
{
	_diffuseMapFileName = fileName;
}

void GLTFExporter::setOcclusionMapFileName(const string& fileName)
{
	_occlusionMapFileName = fileName;
}

void GLTFExporter::setNormalMapFileName(const string& fileName)
{
	_normalMapFileName = fileName;
}

void GLTFExporter::enableCompression(bool enable)
{
	_enableCompression = enable;
}

bool GLTFExporter::exportGLB(const string& outputFileName)
{
	filesystem::path p(outputFileName);
	string baseFileName = p.stem().string();

	if (!exportScene(baseFileName)) {
		return false;
	}

	return true;
}

bool GLTFExporter::exportGLTF(const string& outputFileName)
{
	filesystem::path outputFilePath(outputFileName);
	string baseFileName = outputFilePath.stem().string();

	if (!exportScene(baseFileName)) {
		return false;
	}

	filesystem::ofstream gltfStream(baseFileName + ".gltf");
	gltfStream << _gltf.dump(2);

	filesystem::ofstream binStream(baseFileName + ".bin", std::ios_base::binary);
	binStream.write(_bin.data(), _bin.size());

	return true;
}

bool GLTFExporter::exportScene(const string& name)
{
	if (_verbose) {
		cout << "GLTFExporter::exportScene - create buffer" << endl;
	}

	if (_pScene->mNumMeshes < 1) {
		return setError("scene contains no meshes.");
	}

	aiMesh* pMesh = _pScene->mMeshes[0];

	if (!pMesh->HasPositions()) {
		return setError("mesh contains no positions");
	}

	bool exportNormals = pMesh->HasNormals() && _exportNormals;
	bool exportUVs = pMesh->HasTextureCoords(0) && _exportUVs;

	GLTFObject asset;
	asset.createB


	size_t numVertices = pMesh->mNumVertices;
	size_t numIndices = pMesh->mNumFaces * 3;
	size_t numComponents = 3 + (exportNormals ? 3 : 0) + (exportUVs ? 2 : 0);
	size_t bufferSize = numVertices * sizeof(float) * numComponents + numIndices * sizeof(int);

	_bin.resize(bufferSize);
	
	size_t normalsOffset = writePositions(pMesh, 0);
	size_t uvsOffset = exportNormals ? writeNormals(pMesh, normalsOffset) : normalsOffset;
	size_t indicesOffset = exportUVs ? writeUVs(pMesh, uvsOffset) : uvsOffset;
	size_t totalSize = writeIndices(pMesh, indicesOffset);

	// writeIndices() returns 0 in case of an error
	if (totalSize == 0) {
		return false;
	}

	if (totalSize != bufferSize) {
		return setError("buffer size mismatch");
	}

	if (_verbose) {
		cout << "GLTFExporter::exportScene - create glTF" << endl;
	}

	// create glTF
	GLTFObject asset;

	GLTFMesh* pAssetMesh = asset.createMesh();
	GLTFPrimitive& prim = pAssetMesh->createPrimitive(GLTFPrimitiveMode::TRIANGLES);

	GLTFBuffer* pBuffer = asset.createBuffer(bufferSize);
	pBuffer->setUri(name + ".bin");

	GLTFBufferView* pVertexPositionView = asset.createBufferView(pBuffer);
	pVertexPositionView->setTarget(GLTFBufferTarget::ARRAY_BUFFER);
	pVertexPositionView->setView(0, normalsOffset);

	GLTFAccessor* pAccPositions = asset.createAccessor(pVertexPositionView);
	pAccPositions->setRange(numVertices, 0);
	pAccPositions->setType(GLTFAccessorType::VEC3, GLTFAccessorComponent::FLOAT);
	pAccPositions->setMin({ _min[0], _min[1], _min[2] });
	pAccPositions->setMax({ _max[0], _max[1], _max[2] });
	prim.addPositions(pAccPositions);

	if (exportNormals) {
		GLTFBufferView* pVertexNormalsView = asset.createBufferView(pBuffer);
		pVertexNormalsView->setTarget(GLTFBufferTarget::ARRAY_BUFFER);
		pVertexNormalsView->setView(normalsOffset, uvsOffset - normalsOffset);

		GLTFAccessor* pAccNormals = asset.createAccessor(pVertexNormalsView);
		pAccNormals->setRange(numVertices, 0);
		pAccNormals->setType(GLTFAccessorType::VEC3, GLTFAccessorComponent::FLOAT);
		prim.addNormals(pAccNormals);
	}
	if (exportUVs) {
		GLTFBufferView* pVertexUVsView = asset.createBufferView(pBuffer);
		pVertexUVsView->setTarget(GLTFBufferTarget::ARRAY_BUFFER);
		pVertexUVsView->setView(uvsOffset, indicesOffset - uvsOffset);

		GLTFAccessor* pAccUVs = asset.createAccessor(pVertexUVsView);
		pAccUVs->setRange(numVertices, 0);
		pAccUVs->setType(GLTFAccessorType::VEC2, GLTFAccessorComponent::FLOAT);
		prim.addTexCoords(pAccUVs);
	}

	GLTFBufferView* pIndexView = asset.createBufferView(pBuffer);
	pIndexView->setTarget(GLTFBufferTarget::ELEMENT_ARRAY_BUFFER);
	pIndexView->setView(indicesOffset, bufferSize - indicesOffset);

	GLTFAccessor* pAccIndices = asset.createAccessor(pIndexView);
	pAccIndices->setRange(numIndices, 0);
	pAccIndices->setType(GLTFAccessorType::SCALAR, GLTFAccessorComponent::UNSIGNED_INT);
	prim.setIndices(pAccIndices);

	GLTFScene* pScene = asset.createScene(name);
	GLTFMeshNode* pNode = asset.createMeshNode(pAssetMesh, name);
	pScene->addNode(pNode);
	asset.setMainScene(pScene);


	// retrieve material
	int matIndex = pMesh->mMaterialIndex;
	aiMaterial* pMaterial = matIndex >= 0 ? _pScene->mMaterials[matIndex] : nullptr;

	_gltf = asset.toJSON();

	return true;
}

size_t GLTFExporter::writePositions(const aiMesh* pMesh, size_t offset)
{
	if (_verbose) {
		cout << "GLTFExporter::writePositions" << endl;
	}

	_min[0] = _min[1] = _min[2] = std::numeric_limits<float>::max();
	_max[0] = _max[1] = _max[2] = -_min[0];

	size_t numVertices = pMesh->mNumVertices;
	aiVector3D* pSrc = pMesh->mVertices;
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numVertices; ++i) {
		float x = pDst[i*3  ] = pSrc[i].x; _min[0] = min(x, _min[0]); _max[0] = max(x, _max[0]);
		float y = pDst[i*3+1] = pSrc[i].y; _min[1] = min(y, _min[1]); _max[1] = max(y, _max[1]);
		float z = pDst[i*3+2] = pSrc[i].z; _min[2] = min(z, _min[2]); _max[2] = max(z, _max[2]);
	}

	return offset + numVertices * 3 * sizeof(float);
}

size_t GLTFExporter::writeNormals(const aiMesh* pMesh, size_t offset)
{
	if (_verbose) {
		cout << "GLTFExporter::writeNormals" << endl;
	}

	size_t numVertices = pMesh->mNumVertices;
	aiVector3D* pSrc = pMesh->mNormals;
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numVertices; ++i) {
		pDst[i*3  ] = pSrc[i].x;
		pDst[i*3+1] = pSrc[i].y;
		pDst[i*3+2] = pSrc[i].z;
	}

	return offset + numVertices * 3 * sizeof(float);
}

size_t GLTFExporter::writeUVs(const aiMesh* pMesh, size_t offset)
{
	if (_verbose) {
		cout << "GLTFExporter::writeUVs" << endl;
	}

	size_t numVertices = pMesh->mNumVertices;
	aiVector3D* pSrc = pMesh->mTextureCoords[0];
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numVertices; ++i) {
		pDst[i*2  ] = pSrc[i].x;
		pDst[i*2+1] = pSrc[i].y;
	}

	return offset + numVertices * 2 * sizeof(float);
}

size_t GLTFExporter::writeIndices(const aiMesh* pMesh, size_t offset)
{
	if (_verbose) {
		cout << "GLTFExporter::writeIndices" << endl;
	}

	uint32_t numVertices = pMesh->mNumVertices;
	uint32_t numFaces = pMesh->mNumFaces;
	aiFace* pFace = pMesh->mFaces;
	uint32_t* pDst = (uint32_t*)(_bin.data() + offset);

	for (size_t i = 0; i < numFaces; ++i) {
		const aiFace& face = pFace[i];

		if (face.mNumIndices != 3) {
			setError("non-triangular face found");
			return 0;
		}

		uint32_t i0 = face.mIndices[0];
		uint32_t i1 = face.mIndices[1];
		uint32_t i2 = face.mIndices[2];

		if (i0 >= numVertices || i1 >= numVertices || i2 >= numVertices) {
			setError("invalid vertex index found");
			return 0;
		}

		pDst[i*3  ] = i0;
		pDst[i*3+1] = i1;
		pDst[i*3+2] = i2;
	}
	return offset + numFaces * 3 * sizeof(uint32_t);
}

bool GLTFExporter::setError(const string& message)
{
	_lastError = string("GLTFExporter - ") + message;
	return false;
}