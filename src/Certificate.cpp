#include "Certificate.hpp"


Certificate::Certificate(const string& name, const string& content): name(name), content(content){
}


const string Certificate::getName() const{
	return name;
}


const string Certificate::getContent() const{
	return content;
}

string Certificate::generatePDF(const string& workingDirectory,const string& outputDirectory) const{
	//Write .tex into workingDirectory
	ofstream output;
	string completePath = workingDirectory;
	completePath.append(name);
	completePath.append(".tex");
	output.open(completePath, ios::out);
	if(!output){
		stringstream message;
		message << "Error while writing configured .tex file " << completePath;
		throw FileAccessError(message.str());
	}
	output << content;
	output.close();
	
	//Generate pdf file
	//Set names and parameters
	string command = "xelatex";
	string haltOnErrorParameter = "-halt-on-error";
	string inputFileParameter(name);
	inputFileParameter.append(".tex");
	string interactionParameter = "-interaction=batchmode";
	//Fork for latex process
	int childPid;
	if ((childPid = vfork()) == -1) {
		//Error, fork failed
		stringstream message;
		message << "Error while forking, vfork() returned childPID -1";
		throw ForkFailedError(message.str());
	} else if (childPid == 0) {
		//Change into workingDirectory
		chdir(workingDirectory.c_str());
		//Execute latex in child process
		execlp(command.c_str(), command.c_str(), haltOnErrorParameter.c_str(), interactionParameter.c_str(), inputFileParameter.c_str(), NULL);
		//Error, exec returned
		stringstream message;
		message << "Error while starting " << command << ", probably texlive is not installed or not in path";
		throw LatexMissingError(message.str());
	} else {
		//wait for latex to finish
		//TODO timeout
		int status;
		waitpid(childPid, &status, 0);
		//Check if latex was successful
		if(status != EXIT_SUCCESS){
			//Error, latex failed
			stringstream message;
			message << "Error while executing " << command << ", it exited with code " << status;
			throw LatexExecutionError(message.str());
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
		if(!finalPdf){
			stringstream message;
			message << "Error while opening final pdf file " << finalPdfPath;
			throw FileAccessError(message.str());
		}
		ifstream temporaryPdf(temporaryPdfPath , ios::in | ios::binary);
		if(!temporaryPdf){
			stringstream message;
			message << "Error while opening temporary pdf file " << temporaryPdfPath;
			throw FileAccessError(message.str());
		}
		finalPdf << temporaryPdf.rdbuf();
		finalPdf.close();
		temporaryPdf.close();
	}
	
	return finalPdfPath;
}
