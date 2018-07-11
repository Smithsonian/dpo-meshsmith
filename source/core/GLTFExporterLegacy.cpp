/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "GLTFExporterLegacy.h"

#include "gltf/gltf.h"
#include "gltf/GLTFDracoExtension.h"

#include "math/Math.h"
#include "math/Matrix4T.h"

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


////////////////////////////////////////////////////////////////////////////////

static GLTFMimeType _mimeTypeFromExtension(const std::string& filePath)
{
	filesystem::path fp(filePath);
	string ext = fp.extension().string();

	if (ext == "png" || ext == "PNG") {
		return GLTFMimeType::IMAGE_PNG;
	}

	return GLTFMimeType::IMAGE_JPEG;
}

////////////////////////////////////////////////////////////////////////////////

GLTFExporterLegacy::GLTFExporterLegacy(const aiScene* pScene) :
	_pScene(pScene)
{
}

void GLTFExporterLegacy::setOptions(const GLTFExporterLegacyOptions& options)
{
	_options = options;
}

Result GLTFExporterLegacy::exportScene(const string& outputFileName)
{
	filesystem::path outputFilePath(outputFileName);
	string baseFileName = outputFilePath.stem().string();

	aiMesh* pMesh = _pScene->mMeshes[0];

	if (!pMesh->HasPositions()) {
		return Result::error("mesh contains no positions");
	}

	bool exportNormals = pMesh->HasNormals() && _options.exportNormals;
	bool exportUVs = pMesh->HasTextureCoords(0) && _options.exportTexCoords;

	size_t numVertices = pMesh->mNumVertices;
	size_t numIndices = pMesh->mNumFaces * 3;
	size_t numFaces = pMesh->mNumFaces;
	
	// GLTF MESH DATA

	GLTFAsset gltf;

	GLTFMesh* pAssetMesh = gltf.createMesh();
	GLTFPrimitive& prim = pAssetMesh->createPrimitive(GLTFPrimitiveMode::TRIANGLES);

	GLTFScene* pScene = gltf.createScene();
	GLTFMeshNode* pNode = gltf.createMeshNode(pAssetMesh);
	Matrix4f matRotation;
	matRotation.makeRotationYPR(0, Math::deg2rad(-90.0f), 0);
	pNode->setMatrix(matRotation);
	pScene->addNode(pNode);
	gltf.setMainScene(pScene);

	GLTFBuffer* pCompressedBuffer = _options.useCompression ? gltf.createBuffer() : nullptr;
	GLTFBuffer* pMeshBuffer = gltf.createBuffer();
	GLTFBuffer* pFirstBuffer = pCompressedBuffer ? pCompressedBuffer : pMeshBuffer;

	auto pAccPosition = gltf.createAccessor<float>(GLTFAccessorType::VEC3);
	pAccPosition->addVertexData(pMeshBuffer, (float*)(pMesh->mVertices), numVertices);
	pAccPosition->updateBounds();
	prim.addPositions(pAccPosition);

	if (exportNormals) {
		auto pAccNormals = gltf.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccNormals->addVertexData(pMeshBuffer, (float*)(pMesh->mNormals), numVertices);
		prim.addNormals(pAccNormals);
	}

	if (exportUVs) {
		auto pAccUVs = gltf.createAccessor<float>(GLTFAccessorType::VEC2);
		float* pData = pAccUVs->allocateVertexData(pMeshBuffer, numVertices);
		const aiVector3D* pSrc = pMesh->mTextureCoords[0];

		for (size_t i = 0; i < numVertices; ++i) {
			const aiVector3D& v = pSrc[i];
			pData[i * 2    ] = v.x;
			pData[i * 2 + 1] = 1.0f - v.y;
		}
		prim.addTexCoords(pAccUVs);
	}

	auto pAccIndices = gltf.createAccessor<uint32_t>(GLTFAccessorType::SCALAR);
	uint32_t* pData = pAccIndices->allocateIndexData(pMeshBuffer, numIndices);
	const aiFace* pSrc = pMesh->mFaces;
	
	for (size_t i = 0; i < numFaces; ++i) {
		const aiFace& f = pSrc[i];
		pData[i * 3    ] = f.mIndices[0];
		pData[i * 3 + 1] = f.mIndices[1];
		pData[i * 3 + 2] = f.mIndices[2];
	}

	pAccIndices->bufferView()->setTarget(GLTFBufferViewTarget::ELEMENT_ARRAY_BUFFER);
	prim.setIndices(pAccIndices);

	// DRACO MESH COMPRESSION
	if (_options.useCompression) {
		GLTFDracoExtension* pDraco = new GLTFDracoExtension();
		gltf.addExtension(pDraco, true);
		pDraco->encode(&prim, pCompressedBuffer);
	}

	// GLTF MATERIAL
	auto pMaterial = gltf.createMaterial("default");
	GLTFPBRMetallicRoughness pbr;
	GLTFTexture* pTexture = nullptr;

	if (!_options.diffuseMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexDiffuseView = pFirstBuffer->addImage(_options.diffuseMapFile);
			pTexture = gltf.createTexture(pTexDiffuseView, _mimeTypeFromExtension(_options.diffuseMapFile));
		}
		else {
			pTexture = gltf.createTexture(_options.diffuseMapFile);
		}
		pbr.setBaseColorTexture(pTexture);
	}
	if (!_options.occlusionMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexOcclusionView = pFirstBuffer->addImage(_options.occlusionMapFile);
			pTexture = gltf.createTexture(pTexOcclusionView, _mimeTypeFromExtension(_options.occlusionMapFile));
		}
		else {
			pTexture = gltf.createTexture(_options.occlusionMapFile);
		}
		pMaterial->setOcclusionTexture(pTexture);
	}
	if (!_options.normalMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexNormalView = pFirstBuffer->addImage(_options.normalMapFile);
			pTexture = gltf.createTexture(pTexNormalView, _mimeTypeFromExtension(_options.normalMapFile));
		}
		else {
			pTexture = gltf.createTexture(_options.normalMapFile);
		}
		pMaterial->setNormalTexture(pTexture);
	}

	pMaterial->setPBRMetallicRoughness(pbr);
	prim.setMaterial(pMaterial);

	if (_options.writeGLB) {
		if (!gltf.saveGLB(baseFileName + ".glb")) {
			return Result::error("failed to write GLB file");
		}

		return Result::ok();
	}

	pFirstBuffer->setUri(baseFileName + ".bin");
	pFirstBuffer->save(baseFileName + ".bin");
	//pMeshBuffer->setUri(baseFileName + "_mesh.bin");
	//pMainBuffer->save(baseFileName + "_mesh.bin");

	if (!gltf.saveGLTF(baseFileName + ".gltf", 2)) {
		return Result::error("failed to write glTF file");
	}

	return Result::ok();
}
