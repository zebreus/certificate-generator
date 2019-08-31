#include "Certificate.hpp"


Certificate::Certificate(const string& name, const string& content): name(name), content(content){
}


const string Certificate::getName() const{
	return name;
}


const string Certificate::getContent() const{
	return content;
}

string Certificate::generatePDF(const string& workingDirectory,const string& outputDirectory, const atomic_bool& killswitch) const{
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
	vector<string> arguments;
	if( CONFIG.docker ){
		arguments.push_back("docker");
		arguments.push_back("run");
		arguments.push_back("--rm");
		arguments.push_back("-v");
		string mount = filesystem::canonical( filesystem::path(workingDirectory) ).string();
		mount.append(":/source/");
		arguments.push_back(mount);
		arguments.push_back("-w=/source/");
		arguments.push_back("--network=none");
		arguments.push_back("--security-opt=no-new-privileges");
		arguments.push_back("--ipc=none");
		string memory = "--memory=";
		memory.append( to_string(CONFIG.maxMemoryPerWorker) );
		arguments.push_back(memory);
		string user = "--user=";
		user.append( to_string( getuid() ) );
		arguments.push_back(user);
		arguments.push_back("--cap-drop=ALL");
		arguments.push_back("alpinexetex");
	}
	
	arguments.push_back("xelatex");
	arguments.push_back("-halt-on-error");
	arguments.push_back("-interaction=batchmode");
	arguments.push_back("-no-shell-escape");
	string inputFileArgument(name);
	inputFileArgument.append(".tex");
	arguments.push_back(inputFileArgument);

	char* charguments[50];
	for(unsigned int i = 0; i < arguments.size() ; i++){
		charguments[i] = const_cast<char*>(arguments[i].c_str());
	}
	charguments[arguments.size()] = nullptr;
	
	//Return if killswitch got set
	if(killswitch){
		return "";
	}
	
	//Fork for latex process
	int childPid;
	if ((childPid = vfork()) == -1) {
		//Error, fork failed
		stringstream message;
		message << "Error while forking, vfork() returned childPID -1";
		throw ForkFailedError(message.str());
	} else if (childPid == 0) {
		//If the server is set to docker this effects just the docker run
		//command, not the container.
		
		//Set cpu time limit
		rlimit cpulimit;
		cpulimit.rlim_cur = CONFIG.maxCpuTimePerWorker;
		cpulimit.rlim_max = CONFIG.maxCpuTimePerWorker+1;
		setrlimit(RLIMIT_CPU, &cpulimit);
		
		//Set memory limit
		rlimit memlimit;
		memlimit.rlim_cur = CONFIG.maxMemoryPerWorker;
		memlimit.rlim_max = CONFIG.maxMemoryPerWorker;
		setrlimit(RLIMIT_AS, &memlimit);
		
		//Increase niceness
		nice(5);
		
		//Redirect stdout to /dev/null
		int fd = open("/dev/null",O_WRONLY | O_CREAT, 0666);   // open the file /dev/null
		dup2(fd, 1);
		
		//Change into workingDirectory
		chdir(workingDirectory.c_str());
		
		//Start latex
		execvp(charguments[0], charguments);
		
		//Error, exec returned
		close(fd);
		stringstream message;
		message << "Error while starting " << charguments[0] << ", probably texlive is not installed or not in path";
		throw LatexMissingError(message.str());
	} else {
		//wait for latex to finish
		//TODO timeout

		//Wait until process has finished, or timeout occurred
		int status;
		int result = 0;
		chrono::time_point end = chrono::system_clock::now()+chrono::seconds(CONFIG.workerTimeout);
		chrono::time_point now = chrono::system_clock::now();
		result = waitpid(childPid, &status, WNOHANG);
		while(result == 0){
			this_thread::sleep_for(10ms);
			now = chrono::system_clock::now();
			if(end < now){
				if(end < (now + 2s)){
					kill(childPid, SIGKILL);
				}else{
					kill(childPid, SIGTERM);
				}
			}
			if(killswitch){
				kill(childPid, SIGKILL);
			}
			
			result = waitpid(childPid, &status, WNOHANG);
		}
		
		//Return if killswitch got set
		if(killswitch){
			return "";
		}
		
		if(result < 0){
			stringstream message;
			message << "Error while waiting for " << charguments[0];
			throw LatexExecutionError(message.str());
		}
		
		//Check if latex was successful
		if(status != EXIT_SUCCESS){
			//Error, latex failed
			stringstream message;
			message << "Error while executing " << charguments[0] << ", it exited with code " << status;
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
