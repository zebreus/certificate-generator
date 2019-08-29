#include "Batch.hpp"


Batch::Batch(vector<Student> students, vector<TemplateCertificate> templateCertificates, const string& workingDirectory, const string& outputDirectory):
	students(students), templateCertificates(templateCertificates), workingDirectory(workingDirectory), outputDirectory(outputDirectory){
}


bool Batch::check() const{
	//Check every student with every template
	for(TemplateCertificate templateCertificate: templateCertificates){
		for(Student student: students){
			if(!templateCertificate.checkStudent(student)){
				return false;
			}
		}
	}
	return true;
}


void Batch::generateCertificates(){
	//TODO Zeit messen und wenn nötig Multithreading
	for(TemplateCertificate templateCertificate: templateCertificates){
		for(Student student: students){
			certificates.push_back(templateCertificate.generateCertificate(student));
		}
	}
}


void Batch::outputCertificates(){
	//TODO Zeit messen und wenn nötig Multithreading und WorkingDirectory im Arbeitsspeicher erstellen
	outputFiles.clear();
	for(Certificate certificate : certificates){
		string generatedPDF = certificate.generatePDF(workingDirectory,outputDirectory);
		outputFiles.push_back(generatedPDF);
	}
}


void Batch::executeBatch(){
	generateCertificates();
	outputCertificates();
}


Batch::Batch(json batchConfiguration){
	try{
		//Load students
		cout << "Loading Students" << endl;
		for(json person : batchConfiguration["students"]){
			cout << person["name"] << " " << person["surname"] << endl;
			students.push_back(Student(person));
		}
		
		//Load templates
		cout << "Loading Templates" << endl;
		for(string templateFile:batchConfiguration["templates"]){
			filesystem::path templateFilePath(templateFile);
			if(templateFilePath.is_relative()){
				templateFilePath = batchConfiguration["workingDirectory"].get<string>();
				templateFilePath.append(templateFile);
			}
			cout << "Loading template file " << templateFilePath << endl;
			ifstream input;
			input.open(templateFilePath, ios::in);
			if(!input){
				stringstream message;
				message << "Error reading template file " << templateFile;
				throw FileAccessError(message.str());
			}
			std::string templateCertificateContent( (std::istreambuf_iterator<char>(input) ),
					   (std::istreambuf_iterator<char>()) );
			input.close();
			//TODO Maybe not push everything as global
			templateCertificates.push_back(TemplateCertificate(templateCertificateContent, batchConfiguration));
		}
		
		//Load directories
		outputDirectory = batchConfiguration["outputDirectory"];
		workingDirectory = batchConfiguration["workingDirectory"];
		outputDirectory.append("/");
		workingDirectory.append("/");
		try{
	        filesystem::create_directories(workingDirectory);
	        filesystem::create_directories(outputDirectory);
	    }catch (const std::exception& e){
	        stringstream message;
			message << "Failed to create workingDirectory or outputDirectory";
			throw FileAccessError(message.str());
	    }
		
		//Copy resources to working directory
		cout << "Copying Resources" << endl;
		for(string resourceFile:batchConfiguration["resources"]){
			filesystem::path resourceFilePath(resourceFile);
			if(resourceFilePath.is_relative()){
				resourceFilePath = batchConfiguration["workingDirectory"].get<string>();
				resourceFilePath.append(resourceFile);
			}
			cout << "Loading ressource file " << resourceFile << endl;
			//Get target Path
			filesystem::path targetFilePath(workingDirectory);
			targetFilePath.append(resourceFilePath.filename().string());
			
			//Check if file is already there
			error_code ec;
			if( !filesystem::equivalent(targetFilePath, resourceFilePath, ec) ){
				ifstream input(resourceFilePath, ios::in | ios::binary);
				if(!input){
					stringstream message;
					message << "Error reading resource file " << resourceFilePath.string();
					throw FileAccessError(message.str());
				}
				ofstream output(targetFilePath, ios::out | ios::binary);
				if(!input){
					stringstream message;
					message << "Error writing resource file " << targetFilePath.string();
					throw FileAccessError(message.str());
				}
				output << input.rdbuf();
				output.close();
				input.close();
			}
		}
	}catch(const nlohmann::detail::exception&){
		stringstream message;
		message << "Error accessing json";
		throw InvalidConfigurationError(message.str());
	}
}

Batch::~Batch(){
}

vector<string> Batch::getOutputFiles() const{
	return outputFiles;
}
