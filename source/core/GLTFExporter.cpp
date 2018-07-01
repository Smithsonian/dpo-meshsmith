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


GLTFExporter::GLTFExporter(const aiScene* pScene) :
	_pScene(pScene),
	_exportNormals(true),
	_exportUVs(true),
	_enableCompression(false)
{
}

GLTFExporter::~GLTFExporter()
{
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

	filesystem::ofstream binStream(baseFileName + ".bin");
	binStream.write(_bin.data(), _bin.size());

	return true;
}

bool GLTFExporter::exportScene(const string& name)
{
	if (_pScene->mNumMeshes < 1) {
		return setError("scene contains no meshes.");
	}

	aiMesh* pMesh = _pScene->mMeshes[0];

	if (!pMesh->HasPositions()) {
		return setError("mesh contains no positions");
	}

	bool exportNormals = pMesh->HasNormals() && _exportNormals;
	bool exportUVs = pMesh->HasTextureCoords(0) && _exportUVs;

	size_t numVertices = pMesh->mNumVertices;
	size_t numIndices = pMesh->mNumFaces * 3;
	size_t numComponents = 3 + (exportNormals ? 3 : 0) + (exportUVs ? 2 : 0);
	size_t bufferSize = numVertices * sizeof(float) * numComponents;

	_bin.resize(bufferSize);
	
	size_t positionsOffset = 0;
	size_t normalsOffset = 0;
	size_t uvsOffset = 0;
	size_t indicesOffset = 0;
	size_t totalSize = 0;

	normalsOffset = writePositions(pMesh, positionsOffset);
	uvsOffset = exportNormals ? writeNormals(pMesh, normalsOffset) : normalsOffset;
	indicesOffset = exportUVs ? writeUVs(pMesh, uvsOffset) : uvsOffset;
	totalSize = writeIndices(pMesh, indicesOffset);

	if (totalSize != bufferSize) {
		return setError("buffer size mismatch");
	}

	// create glTF
	GLTFAsset asset(name);

	GLTFBuffer* pBuffer = asset.createBuffer(bufferSize);
	pBuffer->setUri(name + ".bin");
	GLTFBufferView* pVertexView = asset.createBufferView(pBuffer);
	pVertexView->setTarget(GLTFBufferView::ARRAY_BUFFER);
	pVertexView->setView(0, indicesOffset);
	GLTFBufferView* pIndexView = asset.createBufferView(pBuffer);
	pIndexView->setTarget(GLTFBufferView::ELEMENT_ARRAY_BUFFER);
	pIndexView->setView(indicesOffset, bufferSize - indicesOffset);

	GLTFMesh* pAssetMesh = asset.createMesh();
	GLTFPrimitive* pPrim = pAssetMesh->createPrimitive(GLTFPrimitive::TRIANGLES);

	GLTFAccessor* pAccPositions = asset.createAccessor(pVertexView);
	pAccPositions->setAccess(numVertices, 0);
	pAccPositions->setType(GLTFAccessor::VEC3, GLTFAccessor::FLOAT);
	pAccPositions->setMin({ _min[0], _min[1], _min[2] });
	pAccPositions->setMax({ _max[0], _max[1], _max[2] });
	pPrim->addPositions(pAccPositions);

	if (exportNormals) {
		GLTFAccessor* pAccNormals = asset.createAccessor(pVertexView);
		pAccNormals->setAccess(numVertices, normalsOffset);
		pAccNormals->setType(GLTFAccessor::VEC3, GLTFAccessor::FLOAT);
		pPrim->addNormals(pAccNormals);
	}
	if (exportUVs) {
		GLTFAccessor* pAccUVs = asset.createAccessor(pVertexView);
		pAccUVs->setAccess(numVertices, uvsOffset);
		pAccUVs->setType(GLTFAccessor::VEC2, GLTFAccessor::FLOAT);
		pPrim->addTexCoords(pAccUVs);
	}

	GLTFAccessor* pAccIndices = asset.createAccessor(pIndexView);
	pAccIndices->setAccess(numIndices, 0);
	pAccIndices->setType(GLTFAccessor::SCALAR, GLTFAccessor::UNSIGNED_INT);
	pPrim->setIndices(pAccIndices);

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
	_min[0] = _min[1] = _min[2] = std::numeric_limits<float>::max();
	_max[0] = _max[1] = _max[2] = -_min[0];

	size_t numVertices = pMesh->mNumVertices;
	float* pSrc = (float*)pMesh->mVertices;
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numVertices; ++i) {
		float x = pDst[i * 3] = pSrc[i * 3]; _min[0] = min(x, _min[0]); _max[0] = max(x, _max[0]);
		float y = pDst[i*3+1] = pSrc[i*3+1]; _min[1] = min(y, _min[1]); _max[1] = max(y, _max[1]);
		float z = pDst[i*3+2] = pSrc[i*3+2]; _min[2] = min(z, _min[2]); _max[2] = max(z, _max[2]);
	}

	return offset + numVertices * 3 * sizeof(float);
}

size_t GLTFExporter::writeNormals(const aiMesh* pMesh, size_t offset)
{
	size_t numFloats = pMesh->mNumVertices * 3;
	float* pSrc = (float*)pMesh->mNormals;
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numFloats; ++i) {
		pDst[i] = pSrc[i];
	}

	return offset + numFloats * sizeof(float);
}

size_t GLTFExporter::writeUVs(const aiMesh* pMesh, size_t offset)
{
	size_t numVertices = pMesh->mNumVertices;
	float* pSrc = (float*)pMesh->mTextureCoords[0];
	float* pDst = (float*)(_bin.data() + offset);

	for (size_t i = 0; i < numVertices; ++i) {
		pDst[i * 2] = pSrc[i * 3];
		pDst[i * 2 + 1] = pSrc[i * 3 + 1];
	}

	return offset + numVertices * 2 * sizeof(float);
}

size_t GLTFExporter::writeIndices(const aiMesh* pMesh, size_t offset)
{
	size_t numFaces = pMesh->mNumFaces;
	aiFace* pFace = pMesh->mFaces;
	uint32_t* pDst = (uint32_t*)(_bin.data() + offset);

	for (size_t i = 0; i < numFaces; ++i) {
		pDst[i * 3] = pFace[i].mIndices[0];
		pDst[i * 3 + 1] = pFace[i].mIndices[1];
		pDst[i * 3 + 2] = pFace[i].mIndices[2];
	}

	return offset + numFaces * 3 * sizeof(uint32_t);
}

bool GLTFExporter::setError(const string& message)
{
	_lastError = string("GLTFExporter - ") + message;
	return false;
}