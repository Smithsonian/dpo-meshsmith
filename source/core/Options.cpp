/**
* MeshSmith
*
* @author Ralph Wiedemeier <ralph@framefactory.ch>
* @copyright (c) 2018 Frame Factory GmbH.
*/

#include "Options.h"

using namespace meshsmith;
using namespace flow;
using std::string;

Options::Options() :
	verbose(false),
	report(false),
	list(false),
	joinVertices(false),
	stripNormals(false),
	stripTexCoords(false),
	scale(1.0f),
	translate(0.0f, 0.0f, 0.0f),
	useCompression(false),
	objectSpaceNormals(false),
	embedMaps(false),
	positionQuantizationBits(14),
	texCoordsQuantizationBits(12),
	normalsQuantizationBits(10),
	genericQuantizationBits(8),
	compressionLevel(7)
{
}

Result Options::fromJSON(const flow::json& jsonOptions)
{
	try {
		input = jsonOptions.count("input") ? jsonOptions.at("input").get<string>() : string{};
		output = jsonOptions.count("output") ? jsonOptions.at("output").get<string>() : string{};
		format = jsonOptions.count("format") ? jsonOptions.at("format").get<string>() : string{};
		verbose = jsonOptions.count("verbose") ? jsonOptions.at("verbose").get<bool>() : false;
		report = jsonOptions.count("report") ? jsonOptions.at("report").get<bool>() : false;
		list = jsonOptions.count("list") ? jsonOptions.at("list").get<bool>() : false;
		joinVertices = jsonOptions.count("joinVertices") ? jsonOptions.at("joinVertices").get<bool>() : false;
		stripNormals = jsonOptions.count("stripNormals") ? jsonOptions.at("stripNormals").get<bool>() : false;
		stripTexCoords = jsonOptions.count("stripTexCoords") ? jsonOptions.at("stripTexCoords").get<bool>() : false;
		swizzle = jsonOptions.count("swizzle") ? jsonOptions.at("swizzle").get<string>() : string{};
		scale = jsonOptions.count("scale") ? jsonOptions.at("scale").get<float>() : 1.0f;

		if (jsonOptions.count("translate")) {
			auto t = jsonOptions.at("translate");
			translate.x = t.at(0);
			translate.y = t.at(1);
			translate.z = t.at(2);
		}

		if (jsonOptions.count("gltfx")) {
			auto gltfx = jsonOptions["gltfx"];
			useCompression = gltfx.count("useCompression") ? gltfx.at("useCompression").get<bool>() : false;
			diffuseMap = gltfx.count("diffuseMap") ? gltfx.at("diffuseMap").get<string>() : string{};
			occlusionMap = gltfx.count("occlusionMap") ? gltfx.at("occlusionMap").get<string>() : string{};
			normalMap = gltfx.count("normalMap") ? gltfx.at("normalMap").get<string>() : string{};
			objectSpaceNormals = gltfx.count("objectSpaceNormals") ? gltfx.at("objectSpaceNormals").get<bool>() : false;
			embedMaps = gltfx.count("embedMaps") ? gltfx.at("embedMaps").get<bool>() : false;
		}

		if (jsonOptions.count("compression")) {
			auto compression = jsonOptions["compression"];
			positionQuantizationBits = compression.count("positionQuantizationBits") ? compression.at("positionQuantizationBits").get<uint32_t>() : 14;
			texCoordsQuantizationBits = compression.count("texCoordsQuantizationBits") ? compression.at("texCoordsQuantizationBits").get<uint32_t>() : 12;
			normalsQuantizationBits = compression.count("normalsQuantizationBits") ? compression.at("normalsQuantizationBits").get<uint32_t>() : 10;
			genericQuantizationBits = compression.count("genericQuantizationBits") ? compression.at("genericQuantizationBits").get<uint32_t>() : 8;
			compressionLevel = compression.count("compressionLevel") ? compression.at("compressionLevel").get<bool>() : false;
		}
	}
	catch (const std::exception& e) {
		return Result::error(e.what());
	}

	return Result::ok();
}

json Options::toJSON() const
{
	json result = {
		{ "verbose", verbose },
	};

	if (!input.empty()) {
		result["input"] = input;
	}
	if (!output.empty()) {
		result["output"] = output;
	}
	if (!format.empty()) {
		result["format"] = format;
	}
	if (report) {
		result["report"] = report;
	}
	if (list) {
		result["list"] = list;
	}
	if (joinVertices) {
		result["joinVertices"] = joinVertices;
	}
	if (stripNormals) {
		result["stripNormals"] = stripNormals;
	}
	if (stripTexCoords) {
		result["stripTexCoords"] = stripTexCoords;
	}
	if (!swizzle.empty()) {
		result["swizzle"] = swizzle;
	}
	if (scale != 1.0f) {
		result["scale"] = scale;
	}
	if (!translate.allZero()) {
		result["translate"] = translate.toJSON();
	}

	json gltfx;
	if (useCompression) {
		gltfx["useCompression"] = useCompression;
	}
	if (!diffuseMap.empty()) {
		gltfx["diffuseMap"] = diffuseMap;
	}
	if (!occlusionMap.empty()) {
		gltfx["occlusionMap"] = occlusionMap;
	}
	if (!normalMap.empty()) {
		gltfx["normalMap"] = normalMap;
	}
	if (objectSpaceNormals) {
		gltfx["objectSpaceNormals"] = objectSpaceNormals;
	}
	if (embedMaps) {
		gltfx["embedMaps"] = true;
	}

	if (!gltfx.empty()) {
		result["gltfx"] = gltfx;
	}

	if (useCompression) {
		json compression = {
			{ "compressionLevel", compressionLevel }
		};

		if (positionQuantizationBits > 0) {
			compression["positionQuantizationBits"] = positionQuantizationBits;
		}
		if (texCoordsQuantizationBits > 0) {
			compression["texCoordsQuantizationBits"] = texCoordsQuantizationBits;
		}
		if (normalsQuantizationBits > 0) {
			compression["normalsQuantizationBits"] = normalsQuantizationBits;
		}
		if (genericQuantizationBits > 0) {
			compression["genericQuantizationBits"] = genericQuantizationBits;
		}

		result["compression"] = compression;
	}

	return result;
}