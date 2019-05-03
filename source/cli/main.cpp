/**
 * Intermesh CLI
 *
 * @author Ralph Wiedemeier <ralph@framefactory.ch>
 * @copyright (c) 2018 Frame Factory GmbH.
 */

#include "cxxopts.h"

#include "../core/Options.h"
#include "../core/Debug.h"
#include "../core/Engine.h"
#include "../core/Scene.h"

#include "core/ResultT.h"

#include <string>
#include <fstream>
#include <iostream>

using namespace meshsmith;
using namespace flow;

using std::ofstream;
using std::fstream;
using std::string;
using std::cout;
using std::endl;


int main(int argc, char** ppArgv)
{
#if defined(WIN32) && defined(_DEBUG)
	// re-routes debug output on windows to visual studio output window
	//Intermesh::DebugStreamEnabler dse;
#endif

	cxxopts::Options cliOptions(
		"MeshSmith.exe",
		"MeshSmith Mesh Conversion Tool, CLI v0.6.3, 2019-05-03"
	);

	cliOptions.add_options()
		//("positional", "<input file name>, <output file name>", cxxopts::value<std::vector<std::string>>())
		("c,config", "JSON configuration file", cxxopts::value<string>())
		("i,input", "Input file name", cxxopts::value<string>())
		("o,output", "Output file name", cxxopts::value<string>())
		("f,format", "Output file format", cxxopts::value<string>())
		("a,diffusemap", "Diffuse map file (gltfx/glbx only)", cxxopts::value<string>())
		("b,occlusionmap", "Occlusion map file (gltfx/glbx only)", cxxopts::value<string>())
		("m,normalmap", "Normal map file (gltfx/glbx only)", cxxopts::value<string>())
		("e,embedmaps", "Embed map images (gltfx/glbx only)", cxxopts::value<bool>())
		("t,objectspacenormals", "Use object space normals (gltfx/glbx only)", cxxopts::value<bool>())
		("p,compress", "Compress mesh data using Draco (gltfx/glbx only)", cxxopts::value<bool>())
		("j,joinvertices", "Join identical vertices", cxxopts::value<bool>())
		("n,stripnormals", "Strip normals", cxxopts::value<bool>())
		("u,striptexcoords", "Strip texture coords", cxxopts::value<bool>())
		("z,swizzle", "Swizzle coordinates", cxxopts::value<string>())
		("s,scale", "Scale scene by given factor", cxxopts::value<float>())
		("r,report", "Print JSON-formatted report", cxxopts::value<bool>())
		("l,list", "Print JSON-formatted list of export formats", cxxopts::value<bool>())
		("v,verbose", "Print log messages to std out", cxxopts::value<bool>())
		("h,help", "Displays this message");

	meshsmith::Options options;
	int jsonIndent = -1;

	try {
		auto parsed = cliOptions.parse(argc, ppArgv);

		if (parsed.count("help")) {
			cout << cliOptions.help() << endl;
			exit(0);
		}

		options.verbose = (parsed.count("verbose") != 0);
		if (options.verbose) {
			jsonIndent = 4;
		}

		if (parsed.count("config")) {
			string configFilePath = parsed["config"].as<string>();
			fstream inStream(configFilePath, fstream::in);
			string jsonString((std::istreambuf_iterator<char>(inStream)), std::istreambuf_iterator<char>());

			json jsonParsed;
			try {
				jsonParsed = json::parse(jsonString);
			}
			catch (const std::exception& e) {
				cout << Scene::getJsonStatus(string("failed to parse JSON configuration file: ") + configFilePath
					+ ", reason: " + e.what()).dump(jsonIndent);
				exit(1);
			}

			if (options.verbose) {
				cout << "JSON configuration file: " << configFilePath << endl;
				cout << jsonParsed.dump(jsonIndent) << endl;
			}

			options.fromJSON(jsonParsed);
		}

		if (options.verbose) {
			jsonIndent = 4;
		}

		if (options.list || parsed.count("list")) {
			cout << Scene::getJsonExportFormats().dump(jsonIndent) << endl;
			exit(0);
		}

		options.input = parsed.count("input") ? parsed["input"].as<string>() : options.input;
		options.output = parsed.count("output") ? parsed["output"].as<string>() : options.output;
		options.format = parsed.count("format") ? parsed["format"].as<string>() : options.format;

		options.verbose = parsed.count("verbose") || options.verbose;
		options.report = parsed.count("report") || options.report;
		options.joinVertices = parsed.count("joinvertices") || options.joinVertices;
		options.stripNormals = parsed.count("stripnormals") || options.stripNormals;
		options.stripTexCoords = parsed.count("striptexcoords") || options.stripTexCoords;
		options.swizzle = parsed.count("swizzle") ? parsed["swizzle"].as<string>() : options.swizzle;
		options.scale = parsed.count("scale") ? parsed["scale"].as<float>() : options.scale;

		options.useCompression = parsed.count("compress") || options.useCompression;
		options.objectSpaceNormals = parsed.count("objectspacenormals") || options.objectSpaceNormals;
		options.embedMaps = parsed.count("embedmaps") || options.embedMaps;
		options.diffuseMap = parsed.count("diffusemap") ? parsed["diffusemap"].as<string>() : options.diffuseMap;
		options.occlusionMap = parsed.count("occlusionmap") ? parsed["occlusionmap"].as<string>() : options.occlusionMap;
		options.normalMap = parsed.count("normalmap") ? parsed["normalmap"].as<string>() : options.normalMap;

		if (options.input.empty()) {
			cout << Scene::getJsonStatus("missing input file name").dump(jsonIndent);
			exit(1);
		}
	}
	catch (const cxxopts::OptionException& e) {
		cout << Scene::getJsonStatus(string("error while parsing options: ") + e.what());
		exit(1);
	}

	Scene scene;
	scene.setOptions(options);

	Result result = scene.load();
	if (result.isError()) {
		cout << Scene::getJsonStatus(result.message()).dump(jsonIndent) << endl;
		exit(1);
	}

	if (options.report) {

		// if no output file name is given, write report to console
		if (options.output.empty()) {
			cout << scene.getJsonReport().dump(jsonIndent) << endl;
		}
		// otherwise write report to file
		else {
			ofstream outStream(options.output, ofstream::out);
			if (!outStream.is_open()) {
				cout << Scene::getJsonStatus(string("failed to write to: ") + options.output);
				exit(1);
			}

			outStream << scene.getJsonReport().dump();
			outStream.close();
		}

		exit(0);
	}

	result = scene.process();
	if (result.isError()) {
		cout << Scene::getJsonStatus(result.message()).dump(jsonIndent) << endl;
		exit(1);
	}

	result = scene.save();
	if (result.isError()) {
		cout << Scene::getJsonStatus(result.message()).dump(jsonIndent) << endl;
		exit(1);
	}

	cout << Scene::getJsonStatus().dump(jsonIndent);
	exit(0);

}
