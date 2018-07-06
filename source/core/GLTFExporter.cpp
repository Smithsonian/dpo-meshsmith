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
	return false;
}

bool GLTFExporter::exportGLTF(const string& outputFileName)
{
	filesystem::path outputFilePath(outputFileName);
	string baseFileName = outputFilePath.stem().string();

	aiMesh* pMesh = _pScene->mMeshes[0];

	if (!pMesh->HasPositions()) {
		return setError("mesh contains no positions");
	}

	bool exportNormals = pMesh->HasNormals() && _exportNormals;
	bool exportUVs = pMesh->HasTextureCoords(0) && _exportUVs;

	size_t numVertices = pMesh->mNumVertices;
	size_t numIndices = pMesh->mNumFaces * 3;
	size_t numFaces = pMesh->mNumFaces;
	
	GLTFObject gltf;

	GLTFMesh* pAssetMesh = gltf.createMesh();
	GLTFPrimitive& prim = pAssetMesh->createPrimitive(GLTFPrimitiveMode::TRIANGLES);

	GLTFScene* pScene = gltf.createScene();
	GLTFMeshNode* pNode = gltf.createMeshNode(pAssetMesh);
	pScene->addNode(pNode);
	gltf.setMainScene(pScene);

	GLTFBuffer* pBuffer = gltf.createBuffer();
	pBuffer->setUri(baseFileName + ".bin");

	auto pAccPosition = gltf.createAccessor<float>(GLTFAccessorType::VEC3);
	pAccPosition->setData(pBuffer, (float*)(pMesh->mVertices), numVertices);
	pAccPosition->updateBounds();
	prim.addPositions(pAccPosition);

	if (exportNormals) {
		auto pAccNormals = gltf.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccNormals->setData(pBuffer, (float*)(pMesh->mNormals), numVertices);
		prim.addNormals(pAccNormals);
	}

	if (exportUVs) {
		auto pAccUVs = gltf.createAccessor<float>(GLTFAccessorType::VEC2);
		float* pData = pAccUVs->allocate(pBuffer, numVertices);
		const aiVector3D* pSrc = pMesh->mTextureCoords[0];

		for (size_t i = 0; i < numVertices; ++i) {
			const aiVector3D& v = pSrc[i];
			pData[i * 2    ] = v.x;
			pData[i * 2 + 1] = v.y;
		}
		prim.addTexCoords(pAccUVs);
	}

	auto pAccIndices = gltf.createAccessor<uint32_t>(GLTFAccessorType::SCALAR);
	uint32_t* pData = pAccIndices->allocate(pBuffer, numIndices);
	const aiFace* pSrc = pMesh->mFaces;
	
	for (size_t i = 0; i < numFaces; ++i) {
		const aiFace& f = pSrc[i];
		pData[i * 3    ] = f.mIndices[0];
		pData[i * 3 + 1] = f.mIndices[1];
		pData[i * 3 + 2] = f.mIndices[2];
	}

	pAccIndices->bufferView()->setTarget(GLTFBufferViewTarget::ELEMENT_ARRAY_BUFFER);
	prim.setIndices(pAccIndices);


	gltf.save(baseFileName + ".gltf");
	pBuffer->save(baseFileName + ".bin");

	return true;
}

bool GLTFExporter::setError(const string& message)
{
	_lastError = string("GLTFExporter - ") + message;
	return false;
}