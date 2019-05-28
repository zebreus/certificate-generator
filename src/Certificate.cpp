#include "Certificate.hpp"


Certificate::Certificate(const string& name, const string& content): name(name), content(content){
}


const string Certificate::getName() const{
	return name;
}


const string Certificate::getContent() const{
	return content ;
}

void Certificate::generatePDF(const string& workingDirectory,const string& outputDirectory) const{
	//Write .tex into workingDirectory
	ofstream output;
	string completePath = workingDirectory;
	completePath.append(name);
	completePath.append(".tex");
	output.open(completePath, ios::out);
	output << content;
	output.close();
	
	//Generate pdf file
	//Set names and parameters
	string command = "xelatex";
	string workingDirectoryParameter = "-output-directory=";
	workingDirectoryParameter.append(workingDirectory);
	string inputFileParameter(completePath);
	//string interactionParameter = "-interaction=batchmode";
	string interactionParameter = "";
	//Fork for latex process
	int childPid;
	if ((childPid = vfork()) == -1) {
		//Error, fork failed
		cerr << "ERROR: Not able to fork" << endl;
		exit(EXIT_FAILURE);
	} else if (childPid == 0) {
		//Change into workingDirectory
		//TODO Find out if this also changes the working directory for the parent
		chdir(workingDirectory.c_str());
		//Execute latex in child process
		execlp(command.c_str(), command.c_str(), workingDirectoryParameter.c_str(), interactionParameter.c_str(), inputFileParameter.c_str(), NULL);
		//Error, exec returned
		cerr << "Error: xelatex not found" << endl;
		exit(EXIT_FAILURE);
	} else {
		//wait for latex to finish
		int status;
		waitpid(childPid, &status, 0);
		//Check if latex was successful
		if(status != EXIT_SUCCESS){
			//Error, latex failed
			cerr << "Error: xelatex exited with status " << status << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	//Bring pdf to output directory
	//Set names for moving
	string finalPdfPath = outputDirectory;
	finalPdfPath.append(name);
	finalPdfPath.append(".pdf");
	string temporaryPdfPath = workingDirectory;
	temporaryPdfPath.append(name);
	temporaryPdfPath.append(".pdf");
	//Try moving; will fail, if files are on different filesystems
	int moveSuccessful = rename( temporaryPdfPath.c_str() , finalPdfPath.c_str() );
	//If moving failed, copy files instead
	if(moveSuccessful != 0){
		ofstream finalPdf(finalPdfPath, ios::out | ios::binary);
		ifstream temporaryPdf(temporaryPdfPath , ios::in | ios::binary);
		finalPdf << temporaryPdf.rdbuf();
		finalPdf.close();
		temporaryPdf.close();
	}
	
}
