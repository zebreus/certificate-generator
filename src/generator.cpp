#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>

#define DEFAULT_CONFIG "default.json"

using json = nlohmann::json;
using namespace std;

bool verbose = false;
string workingDirectory;
string templateFile;
string batchConfigFile;
string outputDirectory;
string configFile;

struct certificate_t{
	string filename;
	string content;
};

bool replace(string&, const string&, const string&);
void configure(int, const char**);
void loadConfigFile(const string&);
void ensurePath(const string&);

int main(int argc,const char** argv){
	//Configurate generator
	configure(argc, argv);
	
	//Load json batch configuration
	ifstream input;
	input.open(batchConfigFile, ios::in);
	if(!input){
		cerr << "Error reading batch config" << endl;
		exit(EXIT_FAILURE);
	}
	json batch;
	try{
		batch = json::parse(input);
	}catch(const nlohmann::detail::parse_error&){
		cerr << "Invalid json in batch config: " << batchConfigFile << endl;
		exit(EXIT_FAILURE);
	}
	input.close();
	
	//TODO make template certificate_t
	//Load template
	input.open(templateFile, ios::in);
		if(!input){
		cerr << "Error reading template latex" << endl;
		exit(EXIT_FAILURE);
	}
	std::string templateCertificateContent( (std::istreambuf_iterator<char>(input) ),
                       (std::istreambuf_iterator<char>()) );
	input.close();
	
	//TODO Create a better syntax for replacing / find something common online
	//Make simple changes
	replace(templateCertificateContent,"|||signee|||",batch["signee"]);
	
	//Build output texts
	vector<certificate_t> certificates;
	for(json person : batch["people"]){
		//Create certificate
		certificate_t certificate;
		certificate.content = templateCertificateContent;
		
		//Set filename
		certificate.filename = person["surname"];
		certificate.filename.append("_");
		certificate.filename.append(person["name"]);
		certificate.filename.append(".tex");
		
		//Replace single words
		replace(certificate.content,"|||name|||",person["name"]);
		replace(certificate.content,"|||surname|||",person["surname"]);
		replace(certificate.content,"|||date|||",person["date"]);
		
		//Build achievement string
		string achievements;
		for(json achievement : person["achievements"]){
			achievements.append(achievement["name"]);
			achievements.append(" & ");
			achievements.append(achievement["grade"]);
			achievements.append(" \\\\ \\hline ");
		}
		cout << replace(certificate.content,"|||achievements|||",achievements);
		cout << certificate.content << endl;
		
		certificates.push_back(certificate);
	}
	
	//Write .tex files
	ofstream output;
	for( certificate_t cert : certificates){
		string completePath = workingDirectory;
		completePath.append(cert.filename);
		output.open(completePath, ios::out);
		output << cert.content;
		output.close();
	}
	
	//Generate pdfs
	//TODO besser machen
	for( certificate_t cert : certificates){
		string command = "pdflatex -output-directory=";
		command.append(outputDirectory);
		command.append(" ");
		command.append(workingDirectory);
		command.append(cert.filename);
		system(command.c_str());
	}
	
	//not working cleanup
	string remove = "rm ";
	remove.append(outputDirectory);
	remove.append("*.aux ");
	remove.append(outputDirectory);
	remove.append("*.log");
	system(remove.c_str());
}

//Function to replace string in string
//TODO upgrade to replaceall
bool replace(string& work, const string& search, const string& replace){
	size_t start = work.find(search);
	if(start != string::npos){
		work.replace(start,search.length(),replace);
		return true;
	}
	return false;
}

//Function to load initial configuration
void configure(int argc, const char** argv){
	//Load default config
	if(DEFAULT_CONFIG){
		loadConfigFile(DEFAULT_CONFIG);
	}
	
	//Parse arguments
	for(int i = 1;i< argc;i++){
		if(strcmp(argv[i],"-h")==0){
		cout << "Usage:" << endl <<
			"-t template.tex : Specify template file" << endl <<
			"-b batchfile.json : Specify batch file" << endl <<
			"-c configfile.json : Specify config file; any previous arguments will be overwritten" << endl <<
			"-o outputdirectory : Specify output directory" << endl <<
			"-w workingdirectory : Specify working directory" << endl <<
			"-v : verbose" << endl <<
			"-h : help" << endl;
		exit(EXIT_SUCCESS);
		}else if(strcmp(argv[i],"-v")==0){
			verbose = true;
		}else if(strcmp(argv[i],"-t")==0){
			i++;
			if(i < argc){
				templateFile = argv[i];
			}else{
				cerr << "Expected argument after -t" << endl;
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(argv[i],"-b")==0){
			i++;
			if(i < argc){
				batchConfigFile = argv[i];
			}else{
				cerr << "Expected argument after -b" << endl;
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(argv[i],"-c")==0){
			i++;
			if(i < argc){
				loadConfigFile(argv[i]);
			}else{
				cerr << "Expected argument after -c" << endl;
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(argv[i],"-o")==0){
			i++;
			if(i < argc){
				outputDirectory = argv[i];
			}else{
				cerr << "Expected argument after -o" << endl;
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(argv[i],"-w")==0){
			i++;
			if(i < argc){
				workingDirectory = argv[i];
			}else{
				cerr << "Expected argument after -w" << endl;
				exit(EXIT_FAILURE);
			}
		}else{
			cerr << "Invalid argument: " << argv[i] << endl;
			cerr << "Try -h to get help" << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	//Correct paths if end / is missing
	if(outputDirectory.back() != '/'){
		outputDirectory.append("/");
	}
	if(workingDirectory.back() != '/'){
		workingDirectory.append("/");
	}
	cout << outputDirectory << endl;
	//Ensure the existence of working and output directory
	//TODO inform user if its not possible to create directories
	ensurePath(outputDirectory);
	ensurePath(workingDirectory);
	
	//Check if configuration is valid
	if(templateFile == ""){
		cerr << "Error: No template specified" << endl;
		exit(EXIT_FAILURE);
	}
	if(batchConfigFile == ""){
		cerr << "Error: No batch configuration specified" << endl;
		exit(EXIT_FAILURE);
	}
	if(outputDirectory == ""){
		cerr << "Error: No output directory specified" << endl;
		exit(EXIT_FAILURE);
	}
	if(workingDirectory == ""){
		cerr << "Error: No working directory specified" << endl;
		exit(EXIT_FAILURE);
	}
}

void loadConfigFile(const string& filename){
	ifstream input;
	input.open(filename, ios::in);
	if(!input){
		cerr << "Error opening config file: " << filename << endl;
		exit(EXIT_FAILURE);
	}
	json config;
	try{
		config = json::parse(input);
	}catch(const nlohmann::detail::parse_error&){
		cerr << "Invalid json in config: " << filename << endl;
		exit(EXIT_FAILURE);
	}
	input.close();
	
	//TODO catch nlohmann::detail::type_error
	if(config["outputDirectory"]!=nullptr){
		outputDirectory = config["outputDirectory"];
	}
	if(config["workingDirectory"]!=nullptr){
		workingDirectory = config["workingDirectory"];
	}
	if(config["verbose"]!=nullptr){
		verbose = config["verbose"];
	}
	if(config["batchConfigFile"]!=nullptr){
		batchConfigFile = config["batchConfigFile"];
	}
	if(config["templateFile"]!=nullptr){
		templateFile = config["templateFile"];
	}
}

//Ensure that a path exists
void ensurePath(const string& path){
	//TODO find a better solution without shell
	string command = "mkdir -p -m=750 ";
	command.append(path);
	system(command.c_str());
}
