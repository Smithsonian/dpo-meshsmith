/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "GLTFExporter.h"

#include "gltf/gltf.h"
#include "gltf/GLTFDracoExtension.h"

#include "math/Math.h"
#include "math/Matrix4T.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>

#pragma warning(push)
#pragma warning(disable:4267)
#include "draco/mesh/mesh.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"
#pragma warning(pop)

#include <iostream>

#include "path.h"
#ifdef max
#undef max
#endif


using namespace meshsmith;
using namespace flow;

using draco::GeometryAttribute;
using std::string;
using std::cout;
using std::endl;

////////////////////////////////////////////////////////////////////////////////

static GLTFMimeType _mimeTypeFromExtension(const std::string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	string extenstion = filePath.substr(dotPos + 1);

	if (extenstion == "png" || extenstion == "PNG") {
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

void GLTFExporter::setOptions(const GLTFExporterOptions& options)
{
	_options = options;
}

Result GLTFExporter::exportScene(const aiScene* pAiScene, const string& filePathName)
{
	path filePath(filePathName);

	string fileName = filePath.filename();
	string extension = filePath.extension();
	string fileNameNoExt = fileName.substr(0, fileName.size() - extension.size() - 1);

	uint32_t numMeshes = pAiScene->mNumMeshes;
	if (numMeshes < 1) {
		return Result::error("scene contains no meshes");
	}

	// for now, export first mesh
	GLTFAsset asset;
	asset.setGenerator("MeshSmith mesh conversion tool");

	GLTFBuffer* pBuffer = asset.createBuffer();

	auto meshResult = _exportMesh(pAiScene, 0, asset, pBuffer);
	if (meshResult.isError()) {
		return meshResult;
	}

	auto pMesh = meshResult.value();

	auto materialResult = _createDefaultMaterial(asset, pBuffer);
	//auto materialResult = _exportMaterial(pAiScene, 0, asset, pBuffer);
	if (materialResult.isError()) {
		return materialResult;
	}
	pMesh->setMaterial(materialResult.value());

	GLTFScene* pScene = asset.createScene();
	GLTFMeshNode* pNode = asset.createMeshNode(pMesh);
	pScene->addNode(pNode);
	asset.setMainScene(pScene);

	if (_options.writeBinary) {
		string glbFileName = fileNameNoExt + ".glb";
		string glbFilePath = path(filePath.parent_path() / glbFileName).str();
		if (!asset.saveGLB(glbFilePath)) {
			return Result::error("failed to write GLB file: " + glbFilePath);
		}

		return Result::ok();
	}

	string binaryFileName = fileNameNoExt + ".bin";
	string binaryFilePath = path(filePath.parent_path() / binaryFileName).str();
	pBuffer->setUri(binaryFileName);
	pBuffer->save(binaryFilePath);

	string gltfFilePath = path(filePath.parent_path() / (fileNameNoExt + ".gltf")).str();
	if (!asset.saveGLTF(gltfFilePath, 2)) {
		return Result::error("failed to write glTF file: " + gltfFilePath);
	}

	return Result::ok();
}

ResultT<GLTFMesh*> GLTFExporter::_exportMesh(
	const aiScene* pAiScene, size_t meshIndex, GLTFAsset& asset, GLTFBuffer* pBuffer)
{
	const aiMesh* pAiMesh = pAiScene->mMeshes[meshIndex];

	if (!pAiMesh->HasPositions()) {
		return Result::error(string("mesh contains no positions: ") + pAiMesh->mName.C_Str());
	}

	GLTFMesh* pMesh = asset.createMesh();
	GLTFPrimitive& primitive = pMesh->createPrimitive(GLTFPrimitiveMode::TRIANGLES);
	size_t numVertices = pAiMesh->mNumVertices;

	if (_options.useCompression) {
		GLTFDracoExtension* pDracoExtension = new GLTFDracoExtension();
		asset.addExtension(pDracoExtension, true);
		primitive.addExtension(pDracoExtension);

		Result result = _dracoCompressMesh(pAiMesh, pDracoExtension, pBuffer);
		if (result.isError()) {
			return result;
		}

		auto pAccPosition = asset.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccPosition->setElementCount(numVertices);
		pAccPosition->updateBounds((float*)(pAiMesh->mVertices));
		primitive.addPositions(pAccPosition);

		if (pAiMesh->HasNormals() && !_options.stripNormals) {
			auto pAccNormals = asset.createAccessor<float>(GLTFAccessorType::VEC3);
			pAccNormals->setElementCount(numVertices);
			primitive.addNormals(pAccNormals);
		}

		if (pAiMesh->HasTextureCoords(0) && !_options.stripTexCoords) {
			size_t numComponents = pAiMesh->mNumUVComponents[0];
			GLTFAccessorType accType = numComponents == 0 ? GLTFAccessorType::SCALAR : GLTFAccessorType::VEC2;
			auto pAccUVs = asset.createAccessor<float>(accType);
			pAccUVs->setElementCount(numVertices);
			primitive.addAttribute(GLTFAttributeType::TEXCOORD_0, pAccUVs);
		}
		if (pAiMesh->HasTextureCoords(1) && !_options.stripTexCoords) {
			size_t numComponents = pAiMesh->mNumUVComponents[1];
			GLTFAccessorType accType = numComponents == 0 ? GLTFAccessorType::SCALAR : GLTFAccessorType::VEC2;
			auto pAccUVs = asset.createAccessor<float>(accType);
			pAccUVs->setElementCount(numVertices);
			primitive.addAttribute(GLTFAttributeType::TEXCOORD_1, pAccUVs);
		}

		if (pAiMesh->HasFaces()) {
			auto pAccIndices = asset.createAccessor<uint32_t>(GLTFAccessorType::SCALAR);
			pAccIndices->setElementCount(pAiMesh->mNumFaces * 3);
			primitive.setIndices(pAccIndices);
		}
	}
	else {
		auto pAccPosition = asset.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccPosition->addVertexData(pBuffer, (float*)(pAiMesh->mVertices), numVertices);
		pAccPosition->updateBounds();
		primitive.addPositions(pAccPosition);

		if (pAiMesh->HasNormals() && !_options.stripNormals) {
			auto pAccNormals = asset.createAccessor<float>(GLTFAccessorType::VEC3);
			pAccNormals->addVertexData(pBuffer, (float*)(pAiMesh->mNormals), numVertices);
			primitive.addNormals(pAccNormals);
		}

		if (pAiMesh->HasTextureCoords(0) && !_options.stripTexCoords) {
			_exportTexCoords(pAiMesh, asset, primitive, pBuffer, 0);
		}
		if (pAiMesh->HasTextureCoords(1) && !_options.stripTexCoords) {
			_exportTexCoords(pAiMesh, asset, primitive, pBuffer, 1);
		}

		if (pAiMesh->HasFaces()) {
			Result result = pAiMesh->mNumVertices <= 0xffff
				? _exportFaces<uint16_t>(pAiMesh, asset, primitive, pBuffer)
				: _exportFaces<uint32_t>(pAiMesh, asset, primitive, pBuffer);

			if (result.isError()) {
				return result;
			}
		}
	}

	return ResultT<GLTFMesh*>(pMesh);
}

template<typename T>
Result GLTFExporter::_exportFaces(
	const aiMesh* pAiMesh, GLTFAsset& asset, GLTFPrimitive& primitive, GLTFBuffer* pBuffer)
{
	size_t numFaces = pAiMesh->mNumFaces;

	auto pAccIndices = asset.createAccessor<T>(GLTFAccessorType::SCALAR);
	T* pDst = pAccIndices->allocateIndexData(pBuffer, numFaces * 3);
	const aiFace* pSrc = pAiMesh->mFaces;

	for (size_t i = 0; i < numFaces; ++i) {
		const aiFace& f = pSrc[i];
		if (f.mNumIndices != 3) {
			return Result::error("mesh contains non triangular face");
		}
		pDst[i * 3] = f.mIndices[0];
		pDst[i * 3 + 1] = f.mIndices[1];
		pDst[i * 3 + 2] = f.mIndices[2];
	}

	pAccIndices->bufferView()->setTarget(GLTFBufferViewTarget::ELEMENT_ARRAY_BUFFER);
	primitive.setIndices(pAccIndices);

	return Result::ok();
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
				pDst[i * numComponents + 1] = 1.0f - pSrc[i * 3 + 1];
			}
		}
	}
	else {
		pAccUVs = asset.createAccessor<float>(GLTFAccessorType::VEC3);
		pAccUVs->addVertexData(pBuffer, (float*)(pAiMesh->mTextureCoords[channel]), numVertices);
	}

	primitive.addTexCoords(pAccUVs);
}


GLTFExporter::materialResult_t GLTFExporter::_exportMaterial(
	const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, GLTFBuffer* pBuffer)
{
	const aiMesh* pAiMesh = pAiScene->mMeshes[meshIndex];
	const aiMaterial* pAiMaterial = pAiScene->mMaterials[pAiMesh->mMaterialIndex];

	// TODO: Implement

	return Result::error("not implemented yet");
}

GLTFExporter::materialResult_t GLTFExporter::_createDefaultMaterial(GLTFAsset& asset, GLTFBuffer* pBuffer)
{
	GLTFMaterial* pMaterial = asset.createMaterial("default");
	GLTFPBRMetallicRoughness pbr;
	GLTFTexture* pTexture = nullptr;

	if (_options.verbose) {
		cout << "embed maps: " << _options.embedMaps << endl;
	}

	if (!_options.diffuseMapFile.empty()) {
		if (_options.verbose) {
			cout << "diffuse map file: " << _options.diffuseMapFile << endl;
		}
		if (_options.embedMaps) {
			auto pTexDiffuseView = pBuffer->addImage(_options.diffuseMapFile);
			if (!pTexDiffuseView) {
				return Result::error(string("failed to read diffuse map: " + _options.diffuseMapFile));
			}
			pTexture = asset.createTexture(pTexDiffuseView, _mimeTypeFromExtension(_options.diffuseMapFile));
		}
		else {
			pTexture = asset.createTexture(path(_options.diffuseMapFile).filename());
		}
		pbr.setBaseColorTexture(pTexture);
	}
	if (!_options.occlusionMapFile.empty()) {
		if (_options.verbose) {
			cout << "occlusion map file: " << _options.occlusionMapFile << endl;
		}
		if (_options.embedMaps) {
			auto pTexOcclusionView = pBuffer->addImage(_options.occlusionMapFile);
			if (!pTexOcclusionView) {
				return Result::error(string("failed to read occlusion map: " + _options.occlusionMapFile));
			}
			pTexture = asset.createTexture(pTexOcclusionView, _mimeTypeFromExtension(_options.occlusionMapFile));
		}
		else {
			pTexture = asset.createTexture(path(_options.occlusionMapFile).filename());
		}
		pMaterial->setOcclusionTexture(pTexture);
	}
	if (!_options.normalMapFile.empty()) {
		if (_options.verbose) {
			cout << "Normal map file: " << _options.normalMapFile << endl;
		}
		if (_options.embedMaps) {
			auto pTexNormalView = pBuffer->addImage(_options.normalMapFile);
			if (!pTexNormalView) {
				return Result::error(string("failed to read normal map: " + _options.normalMapFile));
			}
			pTexture = asset.createTexture(pTexNormalView, _mimeTypeFromExtension(_options.normalMapFile));
		}
		else {
			pTexture = asset.createTexture(path(_options.normalMapFile).filename());
		}
		pMaterial->setNormalTexture(pTexture);
	}

	pMaterial->setPBRMetallicRoughness(pbr);
	return ResultT<GLTFMaterial*>(pMaterial);
}

Result GLTFExporter::_dracoCompressMesh(
	const aiMesh* pMesh, GLTFDracoExtension* pDracoExtension, GLTFBuffer* pBuffer)
{
	draco::Mesh dracoMesh;

	if (_options.verbose) {
		cout << "Draco Compression: Build Mesh" << endl;
	}

	Result result = _dracoBuildMesh(pMesh, &dracoMesh, pDracoExtension);
	if (result.isError()) {
		return result;
	}

	if (_options.verbose) {
		cout << "Draco Compression: Encode Mesh" << endl;
	}
	draco::EncoderBuffer encoderBuffer;
	draco::Encoder encoder;
	
	int encodingSpeed = flow::max(0, 10 - _options.draco.compressionLevel);
	encoder.SetSpeedOptions(encodingSpeed, encodingSpeed);
	encoder.SetAttributeQuantization(GeometryAttribute::POSITION, _options.draco.positionQuantizationBits);
	encoder.SetAttributeQuantization(GeometryAttribute::NORMAL, _options.draco.normalsQuantizationBits);
	encoder.SetAttributeQuantization(GeometryAttribute::TEX_COORD, _options.draco.texCoordsQuantizationBits);
	encoder.SetAttributeQuantization(GeometryAttribute::GENERIC, _options.draco.genericQuantizationBits);

	if (_options.verbose) {
		cout << "Compression Level: " << _options.draco.compressionLevel << endl;
		cout << "Position Quantization Bits: " << _options.draco.positionQuantizationBits << endl;
		cout << "Normals Quantization Bits: " << _options.draco.normalsQuantizationBits << endl;
		cout << "TexCoords Quantization Bits: " << _options.draco.texCoordsQuantizationBits << endl;
		cout << "Generic Quantization Bits: " << _options.draco.genericQuantizationBits << endl;
	}

	auto encodeStatus = encoder.EncodeMeshToBuffer(dracoMesh, &encoderBuffer);
	if (!encodeStatus.ok()) {
		return Result::error(string("Draco failed to encode mesh: ") + encodeStatus.error_msg());
	}

	if (_options.verbose) {
		cout << "Draco Compression: Decode Mesh" << endl;
	}
	draco::Decoder decoder;
	draco::DecoderBuffer decoderBuffer;
	decoderBuffer.Init(encoderBuffer.data(), encoderBuffer.size());
	auto decodeResult = decoder.DecodeMeshFromBuffer(&decoderBuffer);
	if (!decodeResult.ok()) {
		return Result::error("Draco failed to decode mesh");
	}

	auto& decodedMesh = decodeResult.value();

	GLTFBufferView* pEncodedView = pBuffer->addData(encoderBuffer.data(), encoderBuffer.size());
	pDracoExtension->setEncodedBufferView(pEncodedView);

	return Result::ok();
}

Result GLTFExporter::_dracoBuildMesh(const aiMesh* pMesh, draco::Mesh* pDracoMesh, GLTFDracoExtension* pDracoExtension)
{
	if (pMesh->mPrimitiveTypes != uint32_t(aiPrimitiveType_TRIANGLE)) {
		return Result::error(string("mesh contains non-triangle primitives: ") + pMesh->mName.C_Str());
	}

	uint32_t numVertices = pMesh->mNumVertices;
	uint32_t v2fsize = sizeof(float) * 2;
	uint32_t v3fsize = sizeof(float) * 3;

	uint32_t numFaces = pMesh->mNumFaces;
	uint32_t numPoints = numFaces * 3;

	pDracoMesh->set_num_points(numPoints);
	pDracoMesh->SetNumFaces(numFaces);

	if (_options.verbose) {
		cout << "Adding " << numVertices << " attribute values" << endl;
	}

	GeometryAttribute positionAttribute;
	positionAttribute.Init(GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, v3fsize, 0);
	int posIndex = pDracoMesh->AddAttribute(positionAttribute, false, numVertices);
	auto pPosAttrib = pDracoMesh->attribute(posIndex);
	pPosAttrib->Reset(numVertices);
	pPosAttrib->buffer()->Write(0, pMesh->mVertices, v3fsize * numVertices);
	pDracoExtension->addAttribute(GLTFAttributeType::POSITION, posIndex);
	if (_options.verbose) {
		cout << "Position attribute added" << endl;
	}

	if (pMesh->HasNormals() && !_options.stripNormals) {
		GeometryAttribute normalAttribute;
		normalAttribute.Init(GeometryAttribute::NORMAL, nullptr, 3, draco::DT_FLOAT32, false, v3fsize, 0);
		int normIndex = pDracoMesh->AddAttribute(normalAttribute, false, numVertices);
		auto pNormAttrib = pDracoMesh->attribute(normIndex);
		pNormAttrib->Reset(numVertices);
		pNormAttrib->buffer()->Write(0, pMesh->mNormals, v3fsize * numVertices);
		pDracoExtension->addAttribute(GLTFAttributeType::NORMAL, normIndex);
		if (_options.verbose) {
			cout << "Normal attribute added" << endl;
		}
	}

	if (pMesh->HasTextureCoords(0) && !_options.stripTexCoords) {
		int texIndex = _dracoAddTexCoords(pMesh, pDracoMesh, 0);
		pDracoExtension->addAttribute(GLTFAttributeType::TEXCOORD_0, texIndex);
		if (_options.verbose) {
			cout << "TexCoord 0 attribute added" << endl;
		}
	}
	if (pMesh->HasTextureCoords(1) && !_options.stripTexCoords) {
		int texIndex = _dracoAddTexCoords(pMesh, pDracoMesh, 1);
		pDracoExtension->addAttribute(GLTFAttributeType::TEXCOORD_1, texIndex);
		if (_options.verbose) {
			cout << "TexCoord 1 attribute added" << endl;
		}
	}

	Result result = _dracoAddFaces(pMesh, pDracoMesh);
	if (result.isError()) {
		return result;
	}

#ifdef DRACO_ATTRIBUTE_DEDUPLICATION_SUPPORTED
	pDracoMesh->DeduplicateAttributeValues();
	pDracoMesh->DeduplicatePointIds();
#endif

	return Result::ok();
}

Result GLTFExporter::_dracoAddFaces(const aiMesh* pMesh, draco::Mesh* pDracoMesh)
{
	uint32_t numFaces = pMesh->mNumFaces;

	if (_options.verbose) {
		cout << "Adding " << numFaces << " faces" << endl;
	}

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
		pAttrib->SetExplicitMapping(numFaces * 3);

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

int GLTFExporter::_dracoAddTexCoords(const aiMesh* pMesh, draco::Mesh* pDracoMesh, uint32_t channel)
{
	GeometryAttribute texCoordsAttribute;
	uint32_t numComponents = pMesh->mNumUVComponents[channel];
	uint32_t componentSize = numComponents * sizeof(float);
	uint32_t numVertices = pMesh->mNumVertices;

	texCoordsAttribute.Init(GeometryAttribute::TEX_COORD, nullptr, numComponents, draco::DT_FLOAT32, false, componentSize, 0);
	int index = pDracoMesh->AddAttribute(texCoordsAttribute, false, numVertices);
	auto pTexAttrib = pDracoMesh->attribute(index);
	pTexAttrib->Reset(numVertices);

	// if the assimp mesh's UV channel has 3 components, just copy it to buffer in one go
	if (numComponents == 3) {
		pTexAttrib->buffer()->Write(0, pMesh->mTextureCoords[channel], componentSize * numVertices);
	}
	else {
		// if the UV channel has 1 or 2 components, create a smaller draco buffer and do an interleaved copy
		const float* pSrc = (const float*)pMesh->mTextureCoords[channel];
		float* pDst = (float*)pTexAttrib->buffer()->data();
		for (uint32_t i = 0; i < numVertices; ++i) {
			pDst[i * numComponents] = pSrc[i * 3];
		}
		if (numComponents == 2) {
			for (uint32_t i = 0; i < numVertices; ++i) {
				pDst[i * numComponents + 1] = 1.0f - pSrc[i * 3 + 1];
			}
		}
	}

	return index;
}
