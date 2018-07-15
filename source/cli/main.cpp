/**
 * Intermesh CLI
 *
 * @author Ralph Wiedemeier <ralph@framefactory.ch>
 * @copyright (c) 2018 Frame Factory GmbH.
 */

#include "cxxopts.h"

#include "../core/Debug.h"
#include "../core/Engine.h"
#include "../core/Scene.h"

#include "core/ResultT.h"

#include <string>
#include <iostream>

using namespace meshsmith;
using namespace flow;


int main(int argc, char** ppArgv)
{
#if defined(WIN32) && defined(_DEBUG)
	// re-routes debug output on windows to visual studio output window
	//Intermesh::DebugStreamEnabler dse;
#endif

	cxxopts::Options options(
		"MeshSmith CLI v0.81",
		"Mesh Conversion Tool, Command Line Interface"
	);

	options.add_options()
		//("positional", "<input file name>, <output file name>", cxxopts::value<std::vector<std::string>>())
		("i,input", "Input file name", cxxopts::value<std::string>())
		("o,output", "Output file name", cxxopts::value<std::string>())
		("f,format", "Output file format", cxxopts::value<std::string>())
		("a,diffusemap", "Diffuse map file (gltfx/glbx only)", cxxopts::value<std::string>())
		("b,occlusionmap", "Occlusion map file (gltfx/glbx only)", cxxopts::value<std::string>())
		("m,normalmap", "Normal map file (gltfx/glbx only)", cxxopts::value<std::string>())
		("e,embedmaps", "Embed map images (gltfx/glbx only)", cxxopts::value<bool>())
		("p,draco", "Compress mesh data using Draco (gltfx/glbx only)", cxxopts::value<bool>())
		("j,joinvertices", "Join identical vertices", cxxopts::value<bool>())
		("n,stripnormals", "Strip normals", cxxopts::value<bool>())
		("u,stripuvs", "Strip UVs", cxxopts::value<bool>())
		("z,swizzle", "Swizzle coordinates", cxxopts::value<std::string>())
		("s,scale", "Scale scene by given factor", cxxopts::value<float>())
		("r,report", "Print JSON-formatted report", cxxopts::value<bool>())
		("l,list", "Print JSON-formatted list of export formats", cxxopts::value<bool>())
		("v,verbose", "Print log messages to std out", cxxopts::value<bool>())
		("h,help", "Displays this message");

	try {
		//options.parse_positional({ "input", "output", "positional" });
		auto parsed = options.parse(argc, ppArgv);

		if (parsed.count("help")) {
			std::cout << options.help() << std::endl;
			exit(0);
		}

		if (parsed.count("list")) {
			std::cout << Scene::getJsonExportFormats();
			exit(0);
		}

		bool joinVertices = (parsed.count("joinvertices") != 0);
		bool stripNormals = (parsed.count("stripnormals") != 0);
		bool stripUVs = (parsed.count("stripuvs") != 0);

		bool jsonReport = (parsed.count("report") != 0);
		bool logVerbose = (parsed.count("verbose") != 0);

		bool swizzle = (parsed.count("swizzle") != 0);
		bool scale = (parsed.count("scale") != 0);

		std::string inputFileName{ parsed.count("input") ? parsed["input"].as<std::string>() : "" };
		std::string outputFileName{ parsed.count("output") ? parsed["output"].as<std::string>() : inputFileName };

		if (inputFileName.empty()) {
			std::cout << Scene::getJsonStatus("missing input file name").dump(2);
			exit(1);
		}

		GLTFExporterOptions options;
		options.embedMaps = (parsed.count("embedmaps") != 0);
		options.useCompression = (parsed.count("draco") != 0);
		options.diffuseMapFile = parsed.count("diffusemap") ? parsed["diffusemap"].as<std::string>() : "";
		options.occlusionMapFile = parsed.count("occlusionmap") ? parsed["occlusionmap"].as<std::string>() : "";
		options.normalMapFile = parsed.count("normalmap") ? parsed["normalmap"].as<std::string>() : "";

		Scene scene;
		scene.setVerbose(logVerbose);
		scene.setGLTFOptions(options);

		Result result = scene.load(inputFileName, stripNormals, stripUVs);
		if (result.isError()) {
			std::cout << scene.getJsonStatus(result.message());
			exit(1);
		}

		if (jsonReport) {
			std::cout << scene.getJsonReport();
			exit(0);
		}

		if (swizzle) {
			std::string swizzleOrder = parsed["swizzle"].as<std::string>();
			scene.swizzle(swizzleOrder);
		}
		
		if (scale) {
			float scalingFactor = parsed["scale"].as<float>();
			scene.scale(scalingFactor);
		}

		if (outputFileName.empty()) {
			std::cout << Scene::getJsonStatus("missing output file name");
			exit(1);
		}

		size_t dotPos = outputFileName.find_last_of('.');
		std::string outputFileBaseName = outputFileName.substr(0, dotPos);

		std::string outputFormat = outputFileName.substr(dotPos + 1);
		if (parsed.count("format")) {
			outputFormat = parsed["format"].as<std::string>();
		}

		result = scene.save(outputFileBaseName, outputFormat, joinVertices, stripNormals, stripUVs);
		if (result.isError()) {
			std::cout << Scene::getJsonStatus(result.message()).dump(2);
			exit(1);
		}

		std::cout << Scene::getJsonStatus().dump(2);
		exit(0);
	}
	catch (const cxxopts::OptionException& e) {
		std::cout << Scene::getJsonStatus(std::string("error while parsing options: ") + e.what());
		exit(1);
	}
}
