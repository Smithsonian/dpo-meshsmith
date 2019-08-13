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

		float metallicFactor;
		float roughnessFactor;

		std::string diffuseMapFile;
		std::string occlusionMapFile;
		std::string emissiveMapFile;
		std::string metallicRoughnessMapFile;
		std::string zoneMapFile;
		std::string normalMapFile;

		GLTFDracoOptions draco;

		GLTFExporterOptions() :
			verbose(false),
			embedMaps(false),
			useCompression(false),
			stripNormals(false),
			stripTexCoords(false),
			writeBinary(false),
			metallicFactor(0.1f),
			roughnessFactor(0.8f) { }
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
		typedef flow::ResultT<flow::GLTFMaterial*> materialResult_t;

		flow::ResultT<flow::GLTFMesh*> _exportMesh(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		template<typename T>
		flow::Result _exportFaces(const aiMesh* pAiMesh, flow::GLTFAsset& asset,
			flow::GLTFPrimitive& primitive, flow::GLTFBuffer* pBuffer);
		
		void _exportTexCoords(
			const aiMesh* pAiMesh, flow::GLTFAsset& asset,
			flow::GLTFPrimitive& primitive, flow::GLTFBuffer* pBuffer, int channel);

		materialResult_t _exportMaterial(
			const aiScene* pAiScene, size_t meshIndex, flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		materialResult_t _createDefaultMaterial(flow::GLTFAsset& asset, flow::GLTFBuffer* pBuffer);

		flow::Result _dracoCompressMesh(const aiMesh* pMesh, flow::GLTFDracoExtension* pDracoExtension, flow::GLTFBuffer* pBuffer);
		flow::Result _dracoBuildMesh(const aiMesh* pMesh, draco::Mesh* pDracoMesh, flow::GLTFDracoExtension* pDracoExtension);
		flow::Result _dracoAddFaces(const aiMesh* pMesh, draco::Mesh* pDracoMesh);
		int _dracoAddTexCoords(const aiMesh* pMesh, draco::Mesh* pDracoMesh, uint32_t channel);

		GLTFExporterOptions _options;
	};
}

#endif // _MESHSMITH_GLTFEXPORTER_H
