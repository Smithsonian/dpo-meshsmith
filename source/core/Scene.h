/**
 * 3D Foundation Project
 * Copyright 2019 Smithsonian Institution
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

		flow::json getJsonReport() const;

	private:
		void _dumpMesh(const aiMesh* pMesh) const;

		Assimp::Importer* _pImporter;
		Assimp::Exporter* _pExporter;
		const aiScene* _pScene;

		Options _options;
	};
}

#endif // _MESHSMITH_SCENE_H
