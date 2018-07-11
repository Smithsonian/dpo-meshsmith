/**
 * MeshSmith
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_GLTFEXPORTERLEGACY_H
#define _MESHSMITH_GLTFEXPORTERLEGACY_H
 
#include "library.h"

#include "core/ResultT.h"
#include "core/json.h"

#include <vector>
#include <string>

struct aiMesh;
struct aiScene;

namespace meshsmith
{
	struct GLTFExporterLegacyOptions
	{
		GLTFExporterLegacyOptions() :
			embedMaps(false),
			useCompression(false),
			exportNormals(true),
			exportTexCoords(true),
			writeGLB(false),
			verbose(false)
		{
		}

		bool embedMaps;
		bool useCompression;
		bool exportNormals;
		bool exportTexCoords;
		bool writeGLB;
		bool verbose;

		std::string diffuseMapFile;
		std::string occlusionMapFile;
		std::string normalMapFile;
	};

	class GLTFExporterLegacy
	{
	public:
		GLTFExporterLegacy(const aiScene* pScene);
		virtual ~GLTFExporterLegacy() { }

		void setOptions(const GLTFExporterLegacyOptions& options);
		flow::Result exportScene(const std::string& outputFileName);

	private:
		GLTFExporterLegacyOptions _options;
		const aiScene* _pScene;
	};
}
 
#endif // _MESHSMITH_GLTFEXPORTERLEGACY_H