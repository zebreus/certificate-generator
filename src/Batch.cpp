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
	//Zeit messen und wenn nötig Multithreading
	for(TemplateCertificate templateCertificate: templateCertificates){
		for(Student student: students){
			certificates.push_back(templateCertificate.generateCertificate(student));
		}
	}
}


void Batch::outputCertificates() const{
	//TODO Zeit messen und wenn nötig Multithreading und WorkingDirectory im Arbeitsspeicher erstellen
	for(Certificate certificate : certificates){
		certificate.generatePDF(workingDirectory,outputDirectory);
	}
}


void Batch::executeBatch(){
	generateCertificates();
	outputCertificates();
}


Batch::Batch(json batchConfiguration){
	//Load students
	cout << "Loading Students" << endl;
	for(json person : batchConfiguration["students"]){
		cout << person["name"] << " " << person["surname"] << endl;
		students.push_back(Student(person));
	}
	
	//Load templates
	cout << "Loading Templates" << endl;
	for(string templateFile:batchConfiguration["templates"]){
		cout << templateFile << endl;
		ifstream input;
		input.open(templateFile, ios::in);
		if(!input){
			cerr << "Error reading template file " << templateFile << endl;
			exit(EXIT_FAILURE);
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
	
	//Copy resources to working directory
	cout << "Copying Resources" << endl;
	for(string resourceFile:batchConfiguration["resources"]){
		cout << resourceFile << endl;
		//Get Path
		string filename = resourceFile.substr(resourceFile.rfind("/")+1, string::npos);
		string targetFile = workingDirectory;
		targetFile.append(filename);
		ofstream output(targetFile, ios::out | ios::binary);
		ifstream input(resourceFile, ios::in | ios::binary);
		output << input.rdbuf();
		output.close();
		input.close();
	}
}
