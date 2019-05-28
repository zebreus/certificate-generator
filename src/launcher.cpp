#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Student.hpp"
#include "TemplateCertificate.hpp"
#include "Certificate.hpp"
#include "Batch.hpp"

#define BATCH_CONFIGURATION "***REMOVED***resources/realbatch.json"

using json = nlohmann::json;
using namespace std;


/**
 * Here the batch configuration is loaded and the batch is checked and executed.
 */
int main(int argc, const char** argv){
	
	//Load batch configuration
	ifstream input;
	input.open(BATCH_CONFIGURATION, ios::in);
	if(!input){
		cerr << "Error reading batch config" << endl;
		exit(EXIT_FAILURE);
	}
	json batchConfiguration;
	try{
		batchConfiguration = json::parse(input);
	}catch(const nlohmann::detail::parse_error& error){
		cerr << "Invalid json in batch config: " << BATCH_CONFIGURATION << endl;
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
