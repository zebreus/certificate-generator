#include <cxxopts.hpp>

#include "gen-cpp/CertificateGenerator.h"
#include "gen-cpp/CertificateGenerator_types.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace ::CertificateGeneratorThrift;
using namespace std;

int main(int argc, char **argv) {
	
	//Parse options
	string batchConfigurationFile;
	vector<string> templateFilePaths;
	vector<string> resourceFilePaths;
	string outputDirectory;
	string serverAddress;
	int serverPort;
	bool verbose = false;
	try{
		cxxopts::Options options(argv[0], "Certificate generator client");
		options.add_options()
			("c,configuration", "A configuration file", cxxopts::value<string>(), "FILE")
			("t,templates", "Template files", cxxopts::value<std::vector<string>>(), "FILE")
			("r,resources", "Resource files", cxxopts::value<std::vector<string>>())
			("h,host", "The generator server host", cxxopts::value<string>())
			("p,port", "The generator server port", cxxopts::value<int>())
			("o,output", "Output PDF directory", cxxopts::value<string>()->default_value("./"))
			("v,verbose", "Enable output", cxxopts::value<bool>(verbose))
			("help", "Print help");
		auto result = options.parse(argc, argv);
		if (result.count("help") || result.arguments().size()==0){
			cout << options.help({""}) << std::endl;
			exit(0);
		}
		if (result.count("configuration")){
			batchConfigurationFile = result["configuration"].as<string>();
		}else{
			throw cxxopts::OptionException("No configuration file specified");
		}
		if (result.count("templates")){
			templateFilePaths = result["templates"].as<std::vector<string>>();
		}
		if (result.count("resources")){
			resourceFilePaths = result["resources"].as<std::vector<string>>();
		}
		outputDirectory = result["output"].as<string>();
		if (result.count("host")){
			serverAddress = result["host"].as<string>();
		}else{
			throw cxxopts::OptionException("No host specified");
		}
		if (result.count("port")){
			serverPort = result["port"].as<int>();
			if(serverPort > 65535 || serverPort < 1){
				throw cxxopts::OptionException("Invalid port specified");
			}
		}else{
			throw cxxopts::OptionException("No port specified");
		}
	}catch (const cxxopts::OptionException& e){
		cerr << "Error parsing options: " << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	//Disable cout
	if(!verbose){
		cout.rdbuf(NULL);
	}
	
	//Ensure that the output directory is valid
	try{
		filesystem::create_directories(outputDirectory);
	}catch(const filesystem::filesystem_error& e){
		cerr << "Error creating output directory: " << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	//Load configuration file
	std::cout << "Loading configuration file" << std::endl;
	stringstream file;
	ifstream input;
	input.open(batchConfigurationFile, ios::in);
	if(!input){
		cerr << "Error opening configuration file" << endl;
		exit(EXIT_FAILURE);
	}
	file << input.rdbuf();
	input.close();
	
	//Load template files
	vector<File> templateFiles;
	for(string filepathStr : templateFilePaths){
		filesystem::path filepath(filepathStr);
		File templateFile;
		templateFile.name = filepath.filename().string();
		ifstream fileStream(filepath, ios::in | ios::binary);
		if(!fileStream){
			cerr << "Error opening template file: " << filepath.string() << endl;
			exit(EXIT_FAILURE);
		}
		stringstream content;
		content << fileStream.rdbuf();
		templateFile.content = content.str();
		fileStream.close();
		templateFiles.push_back( templateFile );
	}
	
	vector<File> resourceFiles;
	
	//Initialize thrift connection
	std::shared_ptr<TTransport> socket(new TSocket(serverAddress, serverPort));
	std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	CertificateGeneratorClient client(protocol);
	
	//Open thrift connection
	std::cout << "Connecting to server " << serverAddress << ":" << serverPort << std::endl;
	transport->open();

	//Generate certificates
	std::vector<File> response;
	std::cout << "Adding resource files" << std::endl;
	client.addResourceFiles(resourceFiles);
	std::cout << "Adding template files" << std::endl;
	client.addTemplateFiles(templateFiles);
	std::cout << "Setting configuration" << std::endl;
	client.setConfigurationData(file.str());
	std::cout << "Checking batch" << std::endl;
	client.checkJob();
	std::cout << "Generating certificate" << std::endl;
	client.generateCertificates(response);
	
	//Close thrift connection
	transport->close();
	
	//Write files to disk
	for(File file : response){
		string outputFile = outputDirectory;
		outputFile.append("/").append(file.name);
		std::cout << "Saving certificate to file " << outputFile << std::endl;
		ofstream output(outputFile, ios::out | ios::binary );
		if(!output){
			cerr << "Error opening output file" << endl;
			exit(EXIT_FAILURE);
		}
		output << file.content;
		output.close();
	}
	cout << "All done" << endl;
	return 0;
}
