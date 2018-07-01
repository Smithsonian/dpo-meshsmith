/**
 * Intermesh CLI
 *
 * @author Ralph Wiedemeier <ralph@framefactory.io>
 * @copyright (c) 2018 Frame Factory GmbH.
 */

#include "../core/Debug.h"
#include "../core/Engine.h"
#include "../core/Scene.h"

#include "cxxopts.h"

#include <string>
#include <iostream>

using namespace meshsmith;


int main(int argc, char** ppArgv)
{
#if defined(WIN32) && defined(_DEBUG)
	// re-routes debug output on windows to visual studio output window
	//Intermesh::DebugStreamEnabler dse;
#endif

	cxxopts::Options options(
		"MeshSmith CLI v0.70",
		"Mesh Conversion Tool, Command Line Interface"
	);

	options.add_options()
		("positional", "<input file name>, <output file name>", cxxopts::value<std::vector<std::string>>())
		("i,input", "Input file name", cxxopts::value<std::string>())
		("o,output", "Output file name", cxxopts::value<std::string>())
		("f,format", "Output file format", cxxopts::value<std::string>())
		("j,joinvertices", "Join identical vertices", cxxopts::value<bool>())
		("n,stripnormals", "Strip normals", cxxopts::value<bool>())
		("u,stripuvs", "Strip UVs", cxxopts::value<bool>())
		("z,swizzle", "Swizzle coordinates", cxxopts::value<std::string>())
		("c,center", "Center scene bounding box", cxxopts::value<bool>())
		("s,scale", "Scale scene by given factor", cxxopts::value<float>())
		("r,report", "Print JSON-formatted report", cxxopts::value<bool>())
		("l,list", "Print JSON-formatted list of export formats", cxxopts::value<bool>())
		("v,verbose", "Print log messages to std out", cxxopts::value<bool>())
		("h,help", "Displays this message");

	try {
		options.parse_positional({ "input", "output", "positional" });
		auto result = options.parse(argc, ppArgv);

		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			exit(0);
		}

		if (result.count("list")) {
			std::cout << Scene::getJsonExportFormats();
			exit(0);
		}

		bool joinVertices = (result.count("joinvertices") != 0);
		bool stripNormals = (result.count("stripnormals") != 0);
		bool stripUVs = (result.count("stripuvs") != 0);

		bool jsonReport = (result.count("report") != 0);
		bool logVerbose = (result.count("verbose") != 0);

		bool swizzle = (result.count("swizzle") != 0);
		bool center = (result.count("center") != 0);
		bool scale = (result.count("scale") != 0);

		std::string inputFileName;
		if (result.count("input")) {
			inputFileName = result["input"].as<std::string>();
		}

		if (inputFileName.empty()) {
			std::cout << Scene::getJsonError("missing input file name");
			exit(1);
		}

		Scene scene;
		scene.setVerbose(logVerbose);

		if (!scene.load(inputFileName, stripNormals, stripUVs)) {
			std::cout << scene.getJsonStatus();
			exit(1);
		}

		if (jsonReport) {
			std::cout << scene.getJsonReport();
			exit(0);
		}

		if (swizzle) {
			std::string swizzleOrder = result["swizzle"].as<std::string>();
			scene.swizzle(swizzleOrder);
		}
		
		if (center) {
			scene.center();
		}

		if (scale) {
			float scalingFactor = result["scale"].as<float>();
			scene.scale(scalingFactor);
		}

		std::string outputFileName = inputFileName;
		if (result.count("output")) {
			outputFileName = result["output"].as<std::string>();
		}

		if (outputFileName.empty()) {
			std::cout << Scene::getJsonError("missing output file name");
			exit(1);
		}

		size_t dotPos = outputFileName.find_last_of('.');
		std::string outputFileBaseName = outputFileName.substr(0, dotPos);

		std::string outputFormat = outputFileName.substr(dotPos + 1);
		if (result.count("format")) {
			outputFormat = result["format"].as<std::string>();
		}

		if (!scene.save(outputFileBaseName, outputFormat, joinVertices, stripNormals, stripUVs)) {
			std::cout << scene.getJsonStatus();
			exit(1);
		}

		std::cout << scene.getJsonStatus();
		exit(scene.hasError() ? 1 : 0);
	}
	catch (const cxxopts::OptionException& e) {
		std::cout << Scene::getJsonError(std::string("error while parsing options: ") + e.what());
		exit(1);
	}
}
