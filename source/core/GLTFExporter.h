/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.io>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_GLTFEXPORTER_H
#define _MESHSMITH_GLTFEXPORTER_H

#include "core/ResultT.h"
#include "core/json.h"

struct aiScene;
struct aiMesh;
struct aiNode;
struct aiMaterial;

namespace draco
{
	class Mesh;
	class DataBuffer;
	class EncoderBuffer;
}

namespace flow
{
	class GLTFAsset;
	class GLTFMesh;
	class GLTFPrimitive;
	class GLTFBuffer;
	class GLTFMaterial;
}

namespace meshsmith
{
	struct GLTFExporterOptions
	{
		bool verbose;
		bool embedMaps;
		bool useCompression;
		bool exportNormals;
		bool exportTexCoords;
		bool writeGLB;

		std::string diffuseMapFile;
		std::string occlusionMapFile;
		std::string normalMapFile;

		GLTFExporterOptions() :
			verbose(false),
			embedMaps(false),
			useCompression(false),
			exportNormals(true),
			exportTexCoords(true),
			writeGLB(false) { }
	};

	class GLTFExporter
	{
	protected:
		struct _AttribIndices
		{
			int position;
			int normal;
			int tangent;
			int texCoord0;
			int texCoord1;
			int color0;
			int joints0;
			int weights0;

			_AttribIndices() :
				position(-1), normal(-1), tangent(-1), texCoord0(-1),
				texCoord1(-1), color0(-1), joints0(-1), weights0(-1) { }
		};

	public:
		GLTFExporter();
		virtual ~GLTFExporter();

		flow::Result exportScene(const aiScene* pScene, const std::string& fileName);
		

		void setOptions(const GLTFExporterOptions& options);

	protected:
		flow::ResultT<flow::GLTFMesh*> exportMesh(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset,
			flow::GLTFBuffer* pBuffer, flow::GLTFMaterial* pDefaultMaterial);

		void _exportTexCoords(
			const aiMesh* pAiMesh, flow::GLTFAsset& asset,
			flow::GLTFPrimitive& primitive, flow::GLTFBuffer* pBuffer, int channel);

		flow::GLTFMaterial* _exportMaterial(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset,
			flow::GLTFBuffer* pBuffer, flow::GLTFMaterial* pDefaultMaterial);

		flow::GLTFMaterial* _createDefaultMaterial(flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		flow::Result _dracoCompressMesh(const aiMesh* pMesh, draco::EncoderBuffer* pBuffer);
		flow::Result _dracoBuildMesh(const aiMesh* pMesh, draco::Mesh* pDracoMesh, _AttribIndices* pAttribIndices);
		flow::Result _dracoAddFaces(const aiMesh* pMesh, draco::Mesh* pDracoMesh);
		int _dracoAddTexCoords(const aiMesh* pMesh, draco::Mesh* pDracoMesh, uint32_t channel);
		draco::DataBuffer* _dracoCreateBuffer(const void* pData, size_t byteLength);
		void _dracoCleanup();

		GLTFExporterOptions _options;

		typedef std::vector<draco::DataBuffer*> dracoBufferVec_t;
		dracoBufferVec_t _dracoBuffers;
	};
}

#endif // _MESHSMITH_GLTFEXPORTER_H