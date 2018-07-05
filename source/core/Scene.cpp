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
using namespace std;

namespace meshsmith
{
	struct _sceneImpl_t
	{
		Assimp::Importer importer;
		Assimp::Exporter exporter;

		const aiScene* pScene;
		std::string fileName;
		std::string lastError;
		bool verbose;

		uint32_t refCount;
	};
}

string Scene::getJsonExportFormats()
{
	json jsonFormats = {
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

	jsonFormats["list"] = jsonList;
	return jsonFormats.dump(4);
}

string Scene::getJsonError(const std::string& message)
{
	json jsonStatus = {
		{ "type", "status" },
		{ "status", "error" },
		{ "error", message }
	};

	return jsonStatus.dump(4);
}

Scene::Scene()
{
	_createRef();
}

Scene::Scene(const Scene& other)
{
	_pImpl = other._pImpl;
	_addRef();
}

Scene::~Scene()
{
	_releaseRef();
}

Scene& Scene::operator=(const Scene& other)
{
	if (this == &other)
		return *this;

	_releaseRef();
	_pImpl = other._pImpl;
	_addRef();
	return *this;
}

void Scene::setVerbose(bool enabled)
{
	_pImpl->verbose = enabled;
}

bool Scene::load(const std::string& fileName, bool stripNormals, bool stripUVs)
{
	int removeFlags
		= aiComponent_MATERIALS | aiComponent_TEXTURES | aiComponent_LIGHTS
		| aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS
		| aiComponent_COLORS;

	if (stripNormals) {
		if (_pImpl->verbose) {
			cout << "Strip normals/tangents" << endl;
		}
		removeFlags |= aiComponent_NORMALS | aiComponent_TANGENTS_AND_BITANGENTS;
	}

	if (stripUVs) {
		if (_pImpl->verbose) {
			cout << "Strip UVs" << endl;
		}
		removeFlags |= aiComponent_TEXCOORDS;
	}

	_pImpl->importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeFlags);
	int processFlags = aiProcess_RemoveComponent /* | aiProcess_JoinIdenticalVertices */;

	_pImpl->pScene = _pImpl->importer.ReadFile(fileName, processFlags);

	if (!_pImpl->pScene) {
		std::string errorString = _pImpl->importer.GetErrorString();
		_pImpl->lastError = "failed to read: " + fileName + ", reason: " + errorString;
		return false;
	}

	_pImpl->fileName = fileName;
	return true;
}

bool Scene::save(const std::string& fileName, const std::string& formatId, bool joinVertices, bool stripNormals, bool stripUVs) const
{
	bool verbose = _pImpl->verbose;

	size_t dotPos = fileName.find_last_of(".");
	string baseFileName = fileName.substr(0, dotPos);
	string extension;

	if (formatId == "gltfx") {
		if (verbose) {
			cout << "Export as custom glTF X" << endl;
		}

		aiScene* pSceneCopy = nullptr;
		SceneCombiner::CopyScene(&pSceneCopy, _pImpl->pScene);

		//JoinVerticesProcess proc;
		//proc.Execute(pSceneCopy);


		GLTFExporter exporter(pSceneCopy);
		exporter.setVerbose(verbose);
		if (!exporter.exportGLTF(fileName)) {
			_pImpl->lastError = exporter.lastError();
			if (verbose) {
				cout << "Failed: " << exporter.lastError() << endl;
			}
			delete pSceneCopy;
			return false;
		}

		if (verbose) {
			cout << "Success";
		}

		delete pSceneCopy;
		return true;
	}

	size_t formatCount = aiGetExportFormatCount();
	for (size_t i = 0; i < formatCount; ++i) {
		const aiExportFormatDesc* pDesc = aiGetExportFormatDescription(i);
		if (formatId == pDesc->id) {
			extension = pDesc->fileExtension;
			if (verbose) {
				cout << "Export format: " << pDesc->description << endl;
			}
		}
	}

	if (extension.empty()) {
		_pImpl->lastError = "invalid output format id: " + formatId;
		return false;
	}

	string outputFileName = baseFileName + "." + extension;

	if (verbose) {
		cout << "Output file: " << outputFileName << endl;
	}

	Assimp::Exporter exporter;
	Assimp::ExportProperties exportProps;
	int exportFlags = 0;
	
	if (joinVertices) {
		if (verbose) {
			cout << "Join Identical Vertices" << endl;
		}
		exportFlags |= aiProcess_JoinIdenticalVertices;
	}

	aiReturn result = exporter.Export(_pImpl->pScene, formatId,
		outputFileName, exportFlags, &exportProps);

	if (result != aiReturn::aiReturn_SUCCESS) {
		std::string errorString = exporter.GetErrorString();
		_pImpl->lastError = "failed to write: " + outputFileName + ", reason: " + errorString;
		return false;
	}

	return true;
}

void Scene::swizzle(const std::string& order)
{
	if (_pImpl->verbose) {
		cout << "Swizzle " << order << endl;
	}

	Processor::swizzle(_pImpl->pScene, order);
}

void Scene::center()
{
	if (_pImpl->verbose) {
		Vector3f center = Processor::calculateBoundingBox(_pImpl->pScene).center();
		cout << "Center " << center << endl;
	}

	Processor::center(_pImpl->pScene);
}

void Scene::scale(float factor)
{
	Processor::scale(_pImpl->pScene, factor);
}

string Scene::getJsonReport() const
{
	const aiScene* pScene = _pImpl->pScene;

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

string Scene::getJsonStatus() const
{
	string lastError = _pImpl->lastError;
	string status = lastError.empty() ? "ok" : "error";

	json jsonStatus = {
		{ "type", "status" },
		{ "status", status },
		{ "error", lastError }
	};

	return jsonStatus.dump(4);
}

void Scene::dump() const
{
	cout << "File: " << _pImpl->fileName << endl;
	const aiScene* pScene = _pImpl->pScene;
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
	return _pImpl->pScene != nullptr;
}

bool Scene::hasError() const
{
	return !_pImpl->lastError.empty();
}

const std::string& Scene::getLastError() const
{
	return _pImpl->lastError;
}

void Scene::_createRef()
{
	_pImpl = new _sceneImpl_t();
	_pImpl->refCount = 1;
}

void Scene::_addRef()
{
	if (_pImpl)
		_pImpl->refCount++;
}

void Scene::_releaseRef()
{
	if (_pImpl) {
		_pImpl->refCount--;
		if (_pImpl->refCount == 0) {
			delete _pImpl;
		}

		_pImpl = nullptr;
	}
}
