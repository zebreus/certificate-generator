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
	try{
		cxxopts::Options options(argv[0], " - example command line options");
		options.add_options()("c,configuration", "A configuration file", cxxopts::value<string>(), "FILE")("h, help", "Print help");
		auto result = options.parse(argc, argv);
		if (result.count("help") || result.arguments().size()==0){
			cout << options.help({""}) << std::endl;
			exit(0);
		}
		if (result.count("configuration")){
			batchConfigurationFile = result["c"].as<string>();
		}
	}catch (const cxxopts::OptionException& e){
		cout << "error parsing options: " << e.what() << endl;
		exit(1);
	}

	//Load batch configuration
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
		cout << "OK" << endl;
	}else{
		cout << "Check failed" << endl;
		exit(EXIT_FAILURE);
	}
	
	//Execute batch
	cout << "Executing Batch" << endl;
	batch.executeBatch();
	cout << "All done" << endl;
}
