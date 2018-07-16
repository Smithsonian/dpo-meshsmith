/**
* Intermesh Engine
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "Scene.h"
#include "Processor.h"
#include "GLTFExporter.h"

#include "core/json.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/SceneCombiner.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

using namespace meshsmith;
using namespace Assimp;
using namespace flow;

using std::string;
using std::cout;
using std::endl;


json Scene::getJsonExportFormats()
{
	json result = {
		{ "type", "list" },
		{ "status", "ok" }
	};

	json jsonList = json::array();
	size_t count = aiGetExportFormatCount();

	for (size_t i = 0; i < count; ++i) {
		const aiExportFormatDesc* pDesc = aiGetExportFormatDescription(i);
		jsonList.push_back({
			{ "id", pDesc->id },
			{ "extension", pDesc->fileExtension },
			{ "description", pDesc->description }
		});
	}

	result["list"] = jsonList;
	return result;
}

json Scene::getJsonStatus(const std::string& errorMessage /* = std::string{} */)
{
	bool isError = !errorMessage.empty();

	json result = {
		{ "type", "status" },
		{ "status", isError ? "error" : "ok" }
	};

	if (isError) {
		result["error"] = errorMessage;
	}

	return result;
}

Scene::Scene() :
	_pImporter(new Assimp::Importer()),
	_pExporter(new Assimp::Exporter()),
	_pScene(nullptr)
{
}

Scene::~Scene()
{
	F_SAFE_DELETE(_pImporter);
	F_SAFE_DELETE(_pExporter);
}

void Scene::setOptions(const Options& options)
{
	_options = options;
}

Result Scene::load()
{
	int removeFlags
		= aiComponent_MATERIALS | aiComponent_TEXTURES | aiComponent_LIGHTS
		| aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS
		| aiComponent_COLORS;

	if (_options.stripNormals) {
		if (_options.verbose) {
			cout << "Strip normals/tangents" << endl;
		}
		removeFlags |= aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS;
	}

	if (_options.stripTexCoords) {
		if (_options.verbose) {
			cout << "Strip TexCoords" << endl;
		}
		removeFlags |= aiComponent_TEXCOORDS;
	}

	_pImporter->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);
	int processFlags = aiProcess_RemoveComponent | aiProcess_JoinIdenticalVertices;

	_pScene = _pImporter->ReadFile(_options.input, processFlags);

	if (!_pScene) {
		std::string errorString = _pImporter->GetErrorString();
		return Result::error("failed to read input file: " + _options.input + ", reason: " + errorString);
	}

	return Result::ok();
}

Result Scene::save() const
{
	string outputFilePath = _options.output.empty() ? _options.input : _options.output;
	size_t dotPos = outputFilePath.find_last_of(".");
	string baseFilePath = outputFilePath.substr(0, dotPos);
	string extension;

	if (_options.format == "gltfx" || _options.format == "glbx") {
		bool writeBinary = _options.format == "glbx";
		if (_options.verbose) {
			cout << "Exporting custom glTF, binary: " << writeBinary << endl;
		}

		GLTFExporterOptions gltfOptions;
		gltfOptions.verbose = _options.verbose;
		gltfOptions.diffuseMapFile = _options.diffuseMap;
		gltfOptions.occlusionMapFile = _options.occlusionMap;
		gltfOptions.normalMapFile = _options.normalMap;
		gltfOptions.embedMaps = _options.embedMaps;
		gltfOptions.useCompression = _options.useCompression;
		gltfOptions.stripNormals = _options.stripNormals;
		gltfOptions.stripTexCoords = _options.stripTexCoords;
		gltfOptions.writeBinary = writeBinary;

		GLTFDracoOptions dracoOptions;
		dracoOptions.positionQuantizationBits = _options.positionQuantizationBits;
		dracoOptions.texCoordsQuantizationBits = _options.texCoordsQuantizationBits;
		dracoOptions.normalsQuantizationBits = _options.normalsQuantizationBits;
		dracoOptions.genericQuantizationBits = _options.genericQuantizationBits;
		dracoOptions.compressionLevel = _options.compressionLevel;
		gltfOptions.draco = dracoOptions;

		GLTFExporter exporter;
		exporter.setOptions(gltfOptions);

		Result result = exporter.exportScene(_pScene, outputFilePath);
		if (result.isError()) {
			return result;
		}

		return Result::ok();
	}

	size_t formatCount = aiGetExportFormatCount();
	for (size_t i = 0; i < formatCount; ++i) {
		const aiExportFormatDesc* pDesc = aiGetExportFormatDescription(i);
		if (_options.format == pDesc->id) {
			extension = pDesc->fileExtension;
			if (_options.verbose) {
				cout << "Export format: " << pDesc->description << endl;
			}
		}
	}

	if (extension.empty()) {
		return Result::error("invalid output format id: " + _options.format);
	}

	outputFilePath = baseFilePath + "." + extension;

	if (_options.verbose) {
		cout << "Writing to output file: " << outputFilePath << endl;
	}

	Assimp::Exporter exporter;
	Assimp::ExportProperties exportProps;
	int exportFlags = 0;
	
	if (_options.joinVertices) {
		if (_options.verbose) {
			cout << "Join Identical Vertices" << endl;
		}
		exportFlags |= aiProcess_JoinIdenticalVertices;
	}

	aiReturn result = exporter.Export(_pScene, _options.format,
		outputFilePath, exportFlags, &exportProps);

	if (result != aiReturn::aiReturn_SUCCESS) {
		std::string errorString = exporter.GetErrorString();
		return Result::error("failed to write output file: " + outputFilePath + ", reason: " + errorString);
	}

	return Result::ok();
}

Result Scene::process()
{
	if (!_options.swizzle.empty()) {
		if (_options.verbose) {
			cout << "Swizzle: " << _options.swizzle << endl;
		}
		Processor::swizzle(_pScene, _options.swizzle);
	}

	if (_options.scale != 1.0f) {
		if (_options.verbose) {
			cout << "Scale: " << _options.scale << endl;
		}
		Processor::scale(_pScene, _options.scale);
	}

	if (!_options.translate.allZero()) {
		if (_options.verbose) {
			cout << "Translate: " << _options.translate << endl;
		}
		Processor::translate(_pScene, _options.translate);
	}

	return Result::ok();
}

string Scene::getJsonReport() const
{
	const aiScene* pScene = _pScene;

	json jsonReport = {
		{ "type", "report" },
		{ "status", "ok" }
	};

	Range3f boundingBox = Processor::calculateBoundingBox(pScene);
	Vector3f bbMin = boundingBox.lowerBound();
	Vector3f bbMax = boundingBox.upperBound();
	Vector3f size = boundingBox.size();
	Vector3f center = boundingBox.center();

	json scene = {
		{ "numMeshes", pScene->mNumMeshes },
		{ "numMaterials", pScene->mNumMaterials },
		{ "numTextures", pScene->mNumTextures },
		{ "numLights", pScene->mNumLights },
		{ "numCameras", pScene->mNumCameras },
		{ "numAnimations", pScene->mNumAnimations },
		{ "boundingBox", {
			"min", { bbMin.x, bbMin.y, bbMin.z },
			"max", { bbMax.x, bbMax.y, bbMax.z }
		} },
		{ "size", { size.x, size.y, size.z } },
		{ "center", { center.x, center.y, center.z } }
	};

	json jsonMeshes = json::array();
	size_t numMeshes = pScene->mNumMeshes;

	for (size_t i = 0; i < numMeshes; ++i) {
		const aiMesh* pMesh = pScene->mMeshes[i];
		jsonMeshes.push_back({
			{ "numVertices", pMesh->mNumVertices },
			{ "numFaces", pMesh->mNumFaces },
			{ "hasNormals", pMesh->HasNormals() },
			{ "hasTangentsAndBitangents", pMesh->HasTangentsAndBitangents() },
			{ "hasBones", pMesh->HasBones() },
			{ "numUVChannels", pMesh->GetNumUVChannels() },
			{ "numColorChannels", pMesh->GetNumColorChannels() }
		});
	}

	scene["meshes"] = jsonMeshes;
	jsonReport["report"]["scene"] = scene;

	return jsonReport.dump(4);
}

void Scene::dump() const
{
	cout << "File: " << _options.input << endl;
	const aiScene* pScene = _pScene;
	cout << "  Meshes:     " << pScene->mNumMeshes << endl;
	cout << "  Materials:  " << pScene->mNumMaterials << endl;
	cout << "  Textures:   " << pScene->mNumTextures << endl;
	cout << "  Lights:     " << pScene->mNumLights << endl;
	cout << "  Cameras:    " << pScene->mNumCameras << endl;
	cout << "  Animations: " << pScene->mNumAnimations << endl;
	cout << endl;

	size_t numMeshes = pScene->mNumMeshes;

	for (size_t i = 0; i < numMeshes; ++i) {
		const aiMesh* pMesh = pScene->mMeshes[i];
		cout << "  Mesh #" << i;
		if (pMesh->mName.length > 0) {
			cout << " - " << pMesh->mName.C_Str();
		}
		cout << endl;

		cout << "    Vertices:     " << pMesh->mNumVertices << endl;
		cout << "    Faces         " << pMesh->mNumFaces << endl;
		cout << "    Has Normals:  " << pMesh->HasNormals() << endl;
		cout << "    Has Tangents: " << pMesh->HasTangentsAndBitangents() << endl;
		cout << "    UV Channels:  " << pMesh->GetNumUVChannels() << endl;
		cout << "    Col Channels: " << pMesh->GetNumColorChannels() << endl;
		cout << endl;
	}
}

bool Scene::isValid() const
{
	return _pScene != nullptr;
}
