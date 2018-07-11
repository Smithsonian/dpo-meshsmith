/**
 * Intermesh Engine
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_SCENE_H
#define _MESHSMITH_SCENE_H

#include "library.h"

#include "GLTFExporterLegacy.h"
#include "GLTFExporter.h"
#include "core/ResultT.h"

#include <string>

struct aiMesh;
struct aiScene;


namespace Assimp
{
	class Importer;
	class Exporter;
}

namespace meshsmith
{
	class INTERMESH_ENGINE_EXPORT Scene
	{
	public:
		static flow::json getJsonExportFormats();
		static flow::json getJsonStatus(const std::string& error = std::string{});

		Scene();
		~Scene();

		Scene(const Scene& other) = delete;
		Scene& operator=(const Scene& other) = delete;

	public:
		void setGLTFOptions(const GLTFExporterLegacyOptions& options);
		void setVerbose(bool enabled);

		flow::Result load(const std::string& fileName, bool stripNormals, bool stripUVs);
		flow::Result save(const std::string& fileName, const std::string& formatId, bool joinVertices, bool stripNormals, bool stripUVs) const;

		void swizzle(const std::string& order);
		void center();
		void scale(float factor);

		void dump() const;
		bool isValid() const;

		std::string getJsonReport() const;

	private:
		void _dumpMesh(const aiMesh* pMesh) const;

		Assimp::Importer* _pImporter;
		Assimp::Exporter* _pExporter;

		const aiScene* _pScene;
		std::string _fileName;

		GLTFExporterLegacyOptions _gltfExporterOptions;

		bool _verbose;
	};
}

#endif // _MESHSMITH_SCENE_H