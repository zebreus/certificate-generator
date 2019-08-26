#include <nlohmann/json.hpp>
#include <cxxopts.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Student.hpp"
#include "TemplateCertificate.hpp"
#include "Certificate.hpp"
#include "Batch.hpp"

using json = nlohmann::json;
using namespace std;

cxxopts::ParseResult parse(int argc, char* argv[]);

/**
 * Here the batch configuration is loaded and the batch is checked and executed.
 */
int main(int argc, char** argv){
	
	//Parse options
	string batchConfigurationFile;
	string outputFile;
	bool verbose = false;
	try{
		cxxopts::Options options(argv[0], "Certificate generator");
		options.add_options()
			("c,configuration", "A configuration file", cxxopts::value<string>(), "FILE")
			("o,output", "Output PDF filename", cxxopts::value<string>()->default_value("certificate.pdf"))
			("v,verbose", "Enable output", cxxopts::value<bool>(verbose))
			("h, help", "Print help");
		auto result = options.parse(argc, argv);
		if (result.count("help") || result.arguments().size()==0){
			cout << options.help({""}) << std::endl;
			exit(0);
		}
		if (result.count("configuration")){
			batchConfigurationFile = result["c"].as<string>();
		}else{
			throw cxxopts::OptionException("No configuration file specified");
		}
		outputFile = result["output"].as<string>();
	}catch (const cxxopts::OptionException& e){
		cout << "Error parsing options: " << e.what() << endl;
		exit(1);
	}

	//Disable cout
	if(!verbose){
		cout.rdbuf(NULL);
	}
	
	//Check if the output file is valid
	ofstream checkOutput(outputFile, ios::out | ios::binary );
	if(!checkOutput){
		cerr << "Error opening output file" << endl;
		exit(EXIT_FAILURE);
	}
	checkOutput.close();
	
	//Load batch configuration
	std::cout << "Loading configuration file" << std::endl;
	ifstream input;
	input.open(batchConfigurationFile, ios::in);
	if(!input){
		cerr << "Error reading batch config" << endl;
		exit(EXIT_FAILURE);
	}
	json batchConfiguration;
	try{
		batchConfiguration = json::parse(input);
	}catch(const nlohmann::detail::parse_error& error){
		cerr << "Invalid json in batch config: " << batchConfigurationFile << endl;
		cerr << error.what() << endl;
		exit(EXIT_FAILURE);
	}
	input.close();
	
	//Create batch from configuration
	Batch batch(batchConfiguration);
	
	//Check batch
	cout << "Checking Batch" << endl;
	if(batch.check()){
		cout << "Check succeeded" << endl;
	}else{
		cerr << "Check failed" << endl;
		exit(EXIT_FAILURE);
	}
	
	//Execute batch
	cout << "Executing Batch" << endl;
	batch.executeBatch();
	cout << "All done" << endl;
}
