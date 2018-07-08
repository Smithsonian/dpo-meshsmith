/**
* Intermesh Engine
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
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

json Scene::getJsonStatus(const std::string& error /* = std::string{} */)
{
	json result = {
		{ "type", "status" },
		{ "status", error.empty() ? "ok" : "error" },
		{ "error", error }
	};

	return result;
}

Scene::Scene() :
	_pImporter(new Assimp::Importer()),
	_pExporter(new Assimp::Exporter()),
	_pScene(nullptr),
	_verbose(false)
{
}

Scene::~Scene()
{
	F_SAFE_DELETE(_pImporter);
	F_SAFE_DELETE(_pExporter);
}

void Scene::setGLTFOptions(const GLTFExporterOptions& options)
{
	_gltfExporterOptions = options;
}

void Scene::setVerbose(bool enabled)
{
	_verbose = enabled;
}

Result Scene::load(const std::string& fileName, bool stripNormals, bool stripUVs)
{
	int removeFlags
		= aiComponent_MATERIALS | aiComponent_TEXTURES | aiComponent_LIGHTS
		| aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS
		| aiComponent_COLORS;

	if (stripNormals) {
		if (_verbose) {
			cout << "Strip normals/tangents" << endl;
		}
		removeFlags |= aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS;
	}

	if (stripUVs) {
		if (_verbose) {
			cout << "Strip UVs" << endl;
		}
		removeFlags |= aiComponent_TEXCOORDS;
	}

	_pImporter->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);
	int processFlags = aiProcess_RemoveComponent | aiProcess_JoinIdenticalVertices;

	_pScene = _pImporter->ReadFile(fileName, processFlags);

	if (!_pScene) {
		std::string errorString = _pImporter->GetErrorString();
		return Result::error("failed to read: " + fileName + ", reason: " + errorString);
	}

	_fileName = fileName;
	return Result::ok();
}

Result Scene::save(const std::string& fileName, const std::string& formatId, bool joinVertices, bool stripNormals, bool stripUVs) const
{
	size_t dotPos = fileName.find_last_of(".");
	string baseFileName = fileName.substr(0, dotPos);
	string extension;

	if (formatId == "gltfx" || formatId == "glbx") {
		if (_verbose) {
			cout << "Exporting using custom glTF exporter." << endl;
		}

		GLTFExporterOptions options(_gltfExporterOptions);
		options.verbose = _verbose;
		options.writeGLB = (formatId == "glbx");

		GLTFExporter exporter(_pScene);
		exporter.setOptions(options);

		Result result = exporter.exportScene(fileName);
		if (result.isError()) {
			return result;
		}

		return Result::ok();
	}

	size_t formatCount = aiGetExportFormatCount();
	for (size_t i = 0; i < formatCount; ++i) {
		const aiExportFormatDesc* pDesc = aiGetExportFormatDescription(i);
		if (formatId == pDesc->id) {
			extension = pDesc->fileExtension;
			if (_verbose) {
				cout << "Export format: " << pDesc->description << endl;
			}
		}
	}

	if (extension.empty()) {
		return Result::error("invalid output format id: " + formatId);
	}

	string outputFileName = baseFileName + "." + extension;

	if (_verbose) {
		cout << "Output file: " << outputFileName << endl;
	}

	Assimp::Exporter exporter;
	Assimp::ExportProperties exportProps;
	int exportFlags = 0;
	
	if (joinVertices) {
		if (_verbose) {
			cout << "Join Identical Vertices" << endl;
		}
		exportFlags |= aiProcess_JoinIdenticalVertices;
	}

	aiReturn result = exporter.Export(_pScene, formatId,
		outputFileName, exportFlags, &exportProps);

	if (result != aiReturn::aiReturn_SUCCESS) {
		std::string errorString = exporter.GetErrorString();
		return Result::error("failed to write: " + outputFileName + ", reason: " + errorString);
	}

	return Result::ok();
}

void Scene::swizzle(const std::string& order)
{
	if (_verbose) {
		cout << "Swizzle " << order << endl;
	}

	Processor::swizzle(_pScene, order);
}

void Scene::center()
{
	if (_verbose) {
		Vector3f center = Processor::calculateBoundingBox(_pScene).center();
		cout << "Center " << center << endl;
	}

	Processor::center(_pScene);
}

void Scene::scale(float factor)
{
	Processor::scale(_pScene, factor);
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
	cout << "File: " << _fileName << endl;
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
