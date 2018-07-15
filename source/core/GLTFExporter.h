/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_GLTFEXPORTER_H
#define _MESHSMITH_GLTFEXPORTER_H

#include "library.h"
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
	class GLTFDracoExtension;
}

namespace meshsmith
{
	struct GLTFDracoOptions
	{
		int positionQuantizationBits;
		int texCoordsQuantizationBits;
		int normalsQuantizationBits;
		int genericQuantizationBits;
		int compressionLevel;

		GLTFDracoOptions() :
			positionQuantizationBits(14),
			texCoordsQuantizationBits(12),
			normalsQuantizationBits(10),
			genericQuantizationBits(8),
			compressionLevel(7)
		{
		}
	};

	struct GLTFExporterOptions
	{
		bool verbose;
		bool embedMaps;
		bool useCompression;
		bool stripNormals;
		bool stripTexCoords;
		bool writeBinary;

		std::string diffuseMapFile;
		std::string occlusionMapFile;
		std::string normalMapFile;

		GLTFDracoOptions draco;

		GLTFExporterOptions() :
			verbose(false),
			embedMaps(false),
			useCompression(false),
			stripNormals(false),
			stripTexCoords(false),
			writeBinary(false) { }
	};

	class MESHSMITH_CORE_EXPORT GLTFExporter
	{
	public:
		GLTFExporter();
		virtual ~GLTFExporter();

		/// Sets the export options to be used for subsequent calls to exportScene().
		void setOptions(const GLTFExporterOptions& options);

		/// Exports the given Assimp scene to the file with the given name, using
		/// the previously set export options.
		flow::Result exportScene(const aiScene* pScene, const std::string& fileName);

	protected:
		flow::ResultT<flow::GLTFMesh*> _exportMesh(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		template<typename T>
		flow::Result _exportFaces(const aiMesh* pAiMesh, flow::GLTFAsset& asset,
			flow::GLTFPrimitive& primitive, flow::GLTFBuffer* pBuffer);
		
		void _exportTexCoords(
			const aiMesh* pAiMesh, flow::GLTFAsset& asset,
			flow::GLTFPrimitive& primitive, flow::GLTFBuffer* pBuffer, int channel);
		flow::GLTFMaterial* _exportMaterial(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		flow::GLTFMaterial* _createDefaultMaterial(flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		flow::Result _dracoCompressMesh(const aiMesh* pMesh, flow::GLTFDracoExtension* pDracoExtension, flow::GLTFBuffer* pBuffer);
		flow::Result _dracoBuildMesh(const aiMesh* pMesh, draco::Mesh* pDracoMesh, flow::GLTFDracoExtension* pDracoExtension);
		flow::Result _dracoAddFaces(const aiMesh* pMesh, draco::Mesh* pDracoMesh);
		int _dracoAddTexCoords(const aiMesh* pMesh, draco::Mesh* pDracoMesh, uint32_t channel);

		GLTFExporterOptions _options;
	};
}

#endif // _MESHSMITH_GLTFEXPORTER_H