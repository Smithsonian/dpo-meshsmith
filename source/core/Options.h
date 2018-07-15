/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#ifndef _MESHSMITH_OPTIONS_H
#define _MESHSMITH_OPTIONS_H

#include "library.h"
#include "math/Vector3T.h"
#include "core/ResultT.h"
#include "core/json.h"

#include <string>

namespace meshsmith
{
	struct MESHSMITH_CORE_EXPORT Options
	{
		Options();

		flow::Result fromJSON(const flow::json& json);
		flow::json toJSON() const;

		std::string input;
		std::string output;
		std::string format;
		bool verbose;
		bool report;
		bool list;
		bool joinVertices;
		bool stripNormals;
		bool stripTexCoords;
		std::string swizzle;
		float scale;
		flow::Vector3f translate;

		bool useCompression;
		std::string diffuseMap;
		std::string occlusionMap;
		std::string normalMap;
		bool objectSpaceNormals;
		bool embedMaps;

		uint32_t compressionLevel;
		uint32_t positionQuantizationBits;
		uint32_t texCoordsQuantizationBits;
		uint32_t normalsQuantizationBits;
		uint32_t genericQuantizationBits;
	};
}

#endif // _MESHSMITH_OPTIONS_H