/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "GLTFExporter.h"

#include "gltf/gltf.h"
#include "gltf/GLTFDracoExtension.h"

#include "math/Math.h"
#include "math/Matrix4T.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>

#include "draco/mesh/mesh.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"

#include <boost/filesystem.hpp>

using namespace meshsmith;
using namespace flow;
using namespace boost;

using draco::GeometryAttribute;
using std::string;


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

GLTFExporter::GLTFExporter()
{
}

GLTFExporter::~GLTFExporter()
{
}

Result GLTFExporter::exportScene(const aiScene* pAiScene, const string& fileName)
{
	uint32_t numMeshes = pAiScene->mNumMeshes;
	if (numMeshes < 1) {
		return Result::error("scene contains no meshes");
	}

	// for now, export first mesh
	GLTFAsset asset;
	GLTFBuffer* pBuffer = asset.createBuffer();

	auto result = exportMesh(pAiScene, 0, asset, pBuffer, _createDefaultMaterial(asset, pBuffer));
	if (result.isError()) {
		return result;
	}

	GLTFScene* pScene = asset.createScene();
	GLTFMeshNode* pNode = asset.createMeshNode(result.value());
	Matrix4f matRotation;
	matRotation.makeRotationYPR(0, Math::deg2rad(-90.0f), 0);
	pNode->setMatrix(matRotation);
	pScene->addNode(pNode);
	asset.setMainScene(pScene);

	// TODO: Implement

	return Result::ok();
}

ResultT<GLTFMesh*> GLTFExporter::exportMesh(
	const aiScene* pAiScene, size_t meshIndex, GLTFAsset& asset, GLTFBuffer* pBuffer, GLTFMaterial* pDefaultMaterial)
{
	const aiMesh* pAiMesh = pAiScene->mMeshes[meshIndex];

	if (!pAiMesh->HasPositions()) {
		return Result::error(string("mesh contains no positions: ") + pAiMesh->mName.C_Str());
	}

	GLTFMesh* pMesh = asset.createMesh();
	GLTFPrimitive& primitive = pMesh->createPrimitive(GLTFPrimitiveMode::TRIANGLES);
	size_t numVertices = pAiMesh->mNumVertices;

	if (_options.useCompression) {
		draco::EncoderBuffer encoderBuffer;
		Result result = _dracoCompressMesh(pAiMesh, &encoderBuffer);
		if (result.isError()) {
			return result;
		}

		// TODO: Finish
	}
	else {
		auto pAccPosition = asset.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccPosition->addVertexData(pBuffer, (float*)(pAiMesh->mVertices), numVertices);
		pAccPosition->updateBounds();
		primitive.addPositions(pAccPosition);

		if (pAiMesh->HasNormals() && _options.exportNormals) {
			auto pAccNormals = asset.createAccessor<float>(GLTFAccessorType::VEC3);
			pAccNormals->addVertexData(pBuffer, (float*)(pAiMesh->mNormals), numVertices);
			primitive.addNormals(pAccNormals);
		}

		if (pAiMesh->HasTextureCoords(0) && _options.exportTexCoords) {
			_exportTexCoords(pAiMesh, asset, primitive, pBuffer, 0);
		}
		if (pAiMesh->HasTextureCoords(1) && _options.exportTexCoords) {
			_exportTexCoords(pAiMesh, asset, primitive, pBuffer, 1);
		}
	}

	GLTFMaterial* pMaterial = _exportMaterial(pAiScene, meshIndex, asset, pBuffer, pDefaultMaterial);
	primitive.setMaterial(pMaterial);

	return ResultT<GLTFMesh*>(pMesh);
}

void GLTFExporter::setOptions(const GLTFExporterOptions& options)
{
	_options = options;
}

void GLTFExporter::_exportTexCoords(
	const aiMesh* pAiMesh, GLTFAsset& asset, GLTFPrimitive& primitive, GLTFBuffer* pBuffer, int channel)
{
	size_t numVertices = pAiMesh->mNumVertices;
	size_t numComponents = pAiMesh->mNumUVComponents[channel];
	GLTFAccessorT<float>* pAccUVs = nullptr;

	if (numComponents < 3) {
		GLTFAccessorType accType = numComponents == 0 ? GLTFAccessorType::SCALAR : GLTFAccessorType::VEC2;
		pAccUVs = asset.createAccessor<float>(accType);
		const float* pSrc = (const float*)pAiMesh->mTextureCoords[channel];
		float* pDst = pAccUVs->allocateVertexData(pBuffer, numVertices);
		for (size_t i = 0; i < numVertices; ++i) {
			pDst[i * numComponents] = pSrc[i * 3];
		}
		if (numComponents == 2) {
			for (size_t i = 0; i < numVertices; ++i) {
				pDst[i * numComponents + 1] = pSrc[i * 3 + 1];
			}
		}
	}
	else {
		pAccUVs = asset.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccUVs->addVertexData(pBuffer, (float*)(pAiMesh->mTextureCoords[channel]), numVertices);
	}

	primitive.addTexCoords(pAccUVs);
}


GLTFMaterial* GLTFExporter::_exportMaterial(
	const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, GLTFBuffer* pBuffer, GLTFMaterial* pDefaultMaterial)
{
	const aiMesh* pAiMesh = pAiScene->mMeshes[meshIndex];
	const aiMaterial* pAiMaterial = pAiScene->mMaterials[pAiMesh->mMaterialIndex];

	// TODO: Implement

	return pDefaultMaterial;
}

GLTFMaterial* GLTFExporter::_createDefaultMaterial(GLTFAsset& asset, GLTFBuffer* pBuffer)
{
	GLTFMaterial* pMaterial = asset.createMaterial("default");
	GLTFPBRMetallicRoughness pbr;
	GLTFTexture* pTexture = nullptr;

	if (!_options.diffuseMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexDiffuseView = pBuffer->addImage(_options.diffuseMapFile);
			pTexture = asset.createTexture(pTexDiffuseView, _mimeTypeFromExtension(_options.diffuseMapFile));
		}
		else {
			pTexture = asset.createTexture(_options.diffuseMapFile);
		}
		pbr.setBaseColorTexture(pTexture);
	}
	if (!_options.occlusionMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexOcclusionView = pBuffer->addImage(_options.occlusionMapFile);
			pTexture = asset.createTexture(pTexOcclusionView, _mimeTypeFromExtension(_options.occlusionMapFile));
		}
		else {
			pTexture = asset.createTexture(_options.occlusionMapFile);
		}
		pMaterial->setOcclusionTexture(pTexture);
	}
	if (!_options.normalMapFile.empty()) {
		if (_options.embedMaps) {
			auto pTexNormalView = pBuffer->addImage(_options.normalMapFile);
			pTexture = asset.createTexture(pTexNormalView, _mimeTypeFromExtension(_options.normalMapFile));
		}
		else {
			pTexture = asset.createTexture(_options.normalMapFile);
		}
		pMaterial->setNormalTexture(pTexture);
	}

	return pMaterial;
}

Result GLTFExporter::_dracoCompressMesh(const aiMesh* pMesh, draco::EncoderBuffer* pEncoderBuffer)
{
	draco::Mesh dracoMesh;
	_AttribIndices attrIds;

	Result result = _dracoBuildMesh(pMesh, &dracoMesh, &attrIds);
	if (result.isError()) {
		return result;
	}

	draco::Encoder encoder;
	auto encodeStatus = encoder.EncodeMeshToBuffer(dracoMesh, pEncoderBuffer);
	if (!encodeStatus.ok()) {
		return Result::error(string("failed to encode mesh: ") + encodeStatus.error_msg());
	}

	draco::Decoder decoder;
	draco::DecoderBuffer decoderBuffer;
	decoderBuffer.Init(pEncoderBuffer->data(), pEncoderBuffer->size());
	auto decodeResult = decoder.DecodeMeshFromBuffer(&decoderBuffer);
	if (!decodeResult.ok()) {
		return Result::error("failed to decode mesh");
	}

	auto& decodedMesh = decodeResult.value();

	return Result::ok();
}

Result GLTFExporter::_dracoBuildMesh(const aiMesh* pMesh, draco::Mesh* pDracoMesh, _AttribIndices* pAttribIndices)
{
	if (pMesh->mPrimitiveTypes != uint32_t(aiPrimitiveType_TRIANGLE)) {
		return Result::error(string("mesh contains non-triangle primitives: ") + pMesh->mName.C_Str());
	}

	uint32_t numVertices = pMesh->mNumVertices;
	uint32_t v2fsize = sizeof(float) * 2;
	uint32_t v3fsize = sizeof(float) * 3;

	GeometryAttribute positionAttribute;
	positionAttribute.Init(GeometryAttribute::POSITION,
		_dracoCreateBuffer(pMesh->mVertices, v3fsize * numVertices),
		3, draco::DT_FLOAT32, false, v3fsize, 0);
	pAttribIndices->position = pDracoMesh->AddAttribute(positionAttribute, false, numVertices);

	if (pMesh->HasNormals() && _options.exportNormals) {
		GeometryAttribute normalAttribute;
		normalAttribute.Init(GeometryAttribute::NORMAL,
			_dracoCreateBuffer(pMesh->mNormals, v3fsize * numVertices),
			3, draco::DT_FLOAT32, false, v3fsize, 0);
		pAttribIndices->normal = pDracoMesh->AddAttribute(normalAttribute, false, numVertices);
	}

	if (pMesh->HasTextureCoords(0) && _options.exportTexCoords) {
		pAttribIndices->texCoord0 = _dracoAddTexCoords(pMesh, pDracoMesh, 0);
	}
	if (pMesh->HasTextureCoords(1) && _options.exportTexCoords) {
		pAttribIndices->texCoord1 = _dracoAddTexCoords(pMesh, pDracoMesh, 1);
	}

	return _dracoAddFaces(pMesh, pDracoMesh);
}

Result GLTFExporter::_dracoAddFaces(const aiMesh* pMesh, draco::Mesh* pDracoMesh)
{
	uint32_t numVertices = pMesh->mNumVertices;
	uint32_t numFaces = pMesh->mNumFaces;
	uint32_t numPoints = numFaces * 3;

	pDracoMesh->set_num_points(numPoints);
	pDracoMesh->SetNumFaces(numFaces);

	for (uint32_t i = 0; i < numFaces; ++i) {
		draco::Mesh::Face face;
		for (uint32_t c = 0; c < 3; ++c) {
			face[c] = 3 * i + c;
		}
		pDracoMesh->SetFace(draco::FaceIndex(i), face);
	}

	int numAttribs = pDracoMesh->num_attributes();
	for (int i = 0; i < numAttribs; ++i) {
		draco::PointAttribute* pAttrib = pDracoMesh->attribute(i);
		pAttrib->SetExplicitMapping(numVertices);

		for (uint32_t j = 0; j < numFaces; ++j) {
			const aiFace& face = pMesh->mFaces[j];
			if (face.mNumIndices != 3) {
				return Result::error("non-triangular face found. all faces must be triangles.");
			}
			pAttrib->SetPointMapEntry(draco::PointIndex(j * 3), draco::AttributeValueIndex(face.mIndices[0]));
			pAttrib->SetPointMapEntry(draco::PointIndex(j * 3 + 1), draco::AttributeValueIndex(face.mIndices[1]));
			pAttrib->SetPointMapEntry(draco::PointIndex(j * 3 + 2), draco::AttributeValueIndex(face.mIndices[2]));
		}
	}

	return Result::ok();
}

draco::DataBuffer* GLTFExporter::_dracoCreateBuffer(const void* pData, size_t byteLength)
{
	draco::DataBuffer* pBuffer = new draco::DataBuffer();
	_dracoBuffers.push_back(pBuffer);
	pBuffer->Resize(byteLength);
	if (pData) {
		pBuffer->Write(0, pData, byteLength);
	}
	return pBuffer;

}

int GLTFExporter::_dracoAddTexCoords(const aiMesh* pMesh, draco::Mesh* pDracoMesh, uint32_t channel)
{
	GeometryAttribute texCoordsAttribute;
	uint32_t numComponents = pMesh->mNumUVComponents[channel];
	uint32_t componentSize = numComponents * sizeof(float);
	uint32_t numVertices = pMesh->mNumVertices;

	// if the assimp mesh's UV channel has 3 components, just copy it to buffer in one go
	if (numComponents == 3) {
		texCoordsAttribute.Init(GeometryAttribute::TEX_COORD,
			_dracoCreateBuffer(pMesh->mTextureCoords[channel], componentSize * numVertices),
			numComponents, draco::DT_FLOAT32, false, componentSize, 0);
		return pDracoMesh->AddAttribute(texCoordsAttribute, false, numVertices);
	}

	// if the UV channel has 1 or 2 components, create a smaller draco buffer and do an interleaved copy
	auto pBuffer = _dracoCreateBuffer(nullptr, componentSize * numVertices);
	const float* pSrc = (const float*)pMesh->mTextureCoords[channel];
	float* pDst = (float*)pBuffer->data();
	for (uint32_t i = 0; i < numVertices; ++i) {
		pDst[i * numComponents] = pSrc[i * 3];
	}
	if (numComponents == 2) {
		for (uint32_t i = 0; i < numVertices; ++i) {
			pDst[i * numComponents + 1] = pSrc[i * 3 + 1];
		}
	}

	texCoordsAttribute.Init(GeometryAttribute::TEX_COORD,
		pBuffer, numComponents, draco::DT_FLOAT32, false, componentSize, 0);
	return pDracoMesh->AddAttribute(texCoordsAttribute, false, numVertices);
}

void GLTFExporter::_dracoCleanup()
{
	F_SAFE_DELETE_PTR_RANGE(_dracoBuffers);
	_dracoBuffers.clear();
}