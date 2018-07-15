/**
 * Intermesh Engine
 *
 * @author Ralph Wiedemeier <ralph@framefactory.ch>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_SCENE_H
#define _MESHSMITH_SCENE_H

#include "library.h"

#include "GLTFExporter.h"
#include "Options.h"

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
	class MESHSMITH_CORE_EXPORT Scene
	{
	public:
		static flow::json getJsonExportFormats();
		static flow::json getJsonStatus(const std::string& error = std::string{});

		Scene();
		~Scene();

		Scene(const Scene& other) = delete;
		Scene& operator=(const Scene& other) = delete;

	public:
		void setOptions(const Options& options);

		flow::Result load();
		flow::Result process();
		flow::Result save() const;

		void dump() const;
		bool isValid() const;

		std::string getJsonReport() const;

	private:
		void _dumpMesh(const aiMesh* pMesh) const;

		Assimp::Importer* _pImporter;
		Assimp::Exporter* _pExporter;
		const aiScene* _pScene;

		Options _options;
	};
}

#endif // _MESHSMITH_SCENE_H