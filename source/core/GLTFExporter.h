/**
 * MeshSmith
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_GLTFEXPORTER_H
#define _MESHSMITH_GLTFEXPORTER_H
 
#include "library.h"

#include "core/ResultT.h"
#include "core/json.h"

#include <vector>
#include <string>

struct aiMesh;
struct aiScene;

namespace meshsmith
{
	struct GLTFExporterOptions
	{
		GLTFExporterOptions() :
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

	class GLTFExporter
	{
	public:
		GLTFExporter(const aiScene* pScene);
		virtual ~GLTFExporter() { }

		void setOptions(const GLTFExporterOptions& options);
		flow::Result exportScene(const std::string& outputFileName);

	private:
		GLTFExporterOptions _options;
		const aiScene* _pScene;
	};
}
 
#endif // _MESHSMITH_GLTFEXPORTER_H