/**
 * MeshSmith
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */
 
#ifndef _MESHSMITH_GLTFEXPORTER_H
#define _MESHSMITH_GLTFEXPORTER_H
 
#include "library.h"
#include "core/json.h"

#include <vector>
#include <string>

struct aiMesh;
struct aiScene;

namespace meshsmith
{
	class GLTFExporter
	{
	public:
		GLTFExporter(const aiScene* pScene);
		virtual ~GLTFExporter();

		void setVerbose(bool state);
		void setDiffuseMapFileName(const std::string& fileName);
		void setOcclusionMapFileName(const std::string& fileName);
		void setNormalMapFileName(const std::string& fileName);
		void enableCompression(bool enable);

		bool exportGLTF(const std::string& outputFileName);
		bool exportGLB(const std::string& outputFileName);

		const std::string& lastError() const { return _lastError;  }

	protected:
		bool setError(const std::string& message);

	private:
		bool _verbose;

		const aiScene* _pScene;
		std::string _diffuseMapFileName;
		std::string _occlusionMapFileName;
		std::string _normalMapFileName;

		bool _exportNormals;
		bool _exportUVs;
		bool _enableCompression;

		std::string _lastError;
	};
}
 
#endif // _MESHSMITH_GLTFEXPORTER_H