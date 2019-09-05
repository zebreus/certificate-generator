#include "Certificate.hpp"

Certificate::Certificate(const string& name, const string& content)
	: name(name)
	, content(content)
{
}

const string Certificate::getName() const
{
	return name;
}

const string Certificate::getContent() const
{
	return content;
}

void Certificate::writeToWorkingDirectory(const filesystem::path& workingDirectory) const{
	ofstream output;
	filesystem::path completePath(workingDirectory);
	completePath.append(name);
	completePath.replace_extension(".tex");
	output.open(completePath, ios::out);
	if (!output) {
		stringstream message;
		message << "Error while writing configured .tex file " << completePath;
		throw FileAccessError(message.str());
	}
	output << content;
	output.close();
}

filesystem::path Certificate::moveResultToOutputDirectory(const filesystem::path& workingDirectory, const filesystem::path& outputDirectory) const{
	//Set names for moving
	filesystem::path finalPath(outputDirectory);
	finalPath.append(name);
	finalPath.replace_extension(".pdf");
	filesystem::path temporaryPath(workingDirectory);
	temporaryPath.append(name);
	temporaryPath.replace_extension(".pdf");
	//Try moving; will fail, if files are on different filesystems
	int moveSuccessful = rename(temporaryPath.c_str(), finalPath.c_str());
	//If moving failed, copy files instead
	if (moveSuccessful != 0) {
		ofstream finalPdf(finalPath, ios::out | ios::binary);
		if (!finalPdf) {
			stringstream message;
			message << "Error while opening final pdf file " << finalPath;
			throw FileAccessError(message.str());
		}
		ifstream temporaryPdf(temporaryPath, ios::in | ios::binary);
		if (!temporaryPdf) {
			stringstream message;
			message << "Error while opening temporary pdf file " << temporaryPath;
			throw FileAccessError(message.str());
		}
		finalPdf << temporaryPdf.rdbuf();
		finalPdf.close();
		temporaryPdf.close();
	}
	return finalPath;
}

vector<string> Certificate::generateLatexArguments(const filesystem::path& workingDirectory) const{
	vector<string> arguments;
	//If we are using docker we execute latex in a container
	if (CONFIG.docker) {
		arguments.push_back("docker");
		arguments.push_back("run");
		arguments.push_back("--rm");
		arguments.push_back("-v");
		string mount = filesystem::canonical(filesystem::path(workingDirectory)).string();
		mount.append(":/source/");
		arguments.push_back(mount);
		arguments.push_back("-w=/source/");
		arguments.push_back("--network=none");
		arguments.push_back("--security-opt=no-new-privileges");
		arguments.push_back("--ipc=none");
		string memory = "--memory=";
		memory.append(to_string(CONFIG.maxMemoryPerWorker));
		arguments.push_back(memory);
		string user = "--user=";
		user.append(to_string(getuid()));
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
	return arguments;
}

void Certificate::executeProgram(vector<string> arguments, const filesystem::path& workingDirectory) const{
	char* charguments[50];
	for (unsigned int i = 0; i < arguments.size(); i++) {
		charguments[i] = const_cast<char*>(arguments[i].c_str());
	}
	charguments[arguments.size()] = nullptr;
	
	//Set cpu time limit
	rlimit cpulimit;
	cpulimit.rlim_cur = CONFIG.maxCpuTimePerWorker;
	cpulimit.rlim_max = CONFIG.maxCpuTimePerWorker + 1;
	setrlimit(RLIMIT_CPU, &cpulimit);

	//Set memory limit
	rlimit memlimit;
	memlimit.rlim_cur = CONFIG.maxMemoryPerWorker;
	memlimit.rlim_max = CONFIG.maxMemoryPerWorker;
	setrlimit(RLIMIT_AS, &memlimit);

	//Increase niceness
	nice(5);

	//Redirect stdout to /dev/null
	int fd = open("/dev/null", O_WRONLY | O_CREAT, 0666); // open the file /dev/null
	dup2(fd, 1);

	//Change into workingDirectory
	chdir(workingDirectory.c_str());
	
	execvp(charguments[0], charguments);

	//Error, exec returned
	//close(fd);
	stringstream message;
	message << "Error while starting " << charguments[0] << ", probably texlive is not installed or not in path";
	throw LatexMissingError(message.str());
}

int Certificate::waitForProcess(const pid_t& childPid, const atomic_bool& killswitch) const{
	//Wait until process has finished, or timeout occurred
	int status;
	int result = 0;
	chrono::time_point end = chrono::system_clock::now() + chrono::seconds(CONFIG.workerTimeout);
	chrono::time_point now = chrono::system_clock::now();
	result = waitpid(childPid, &status, WNOHANG);
	while (result == 0) {
		this_thread::sleep_for(10ms);
		now = chrono::system_clock::now();
		if (end < now) {
			if (end < (now + 2s)) {
				kill(childPid, SIGKILL);
			} else {
				kill(childPid, SIGTERM);
			}
		}
		if (killswitch) {
			kill(childPid, SIGKILL);
		}

		result = waitpid(childPid, &status, WNOHANG);
	}

	//Error while waiting for child
	if (result < 0) {
		throw LatexExecutionError("Error while waiting for latex");
	}
	
	return status;
}

filesystem::path Certificate::generatePDF(const filesystem::path& workingDirectory, const filesystem::path& outputDirectory, const atomic_bool& killswitch) const
{
	writeToWorkingDirectory(workingDirectory);
	vector<string> arguments = generateLatexArguments(workingDirectory);
	
	if (killswitch) return "";

	//Fork for latex process
	int childPid = vfork();
	if (childPid == -1) {
		throw ForkFailedError("Error while forking, vfork() returned childPID -1");
	} else if (childPid == 0) {
		executeProgram(arguments, workingDirectory);
	} else {
		//Wait until process has finished, or timeout occurred
		int status = waitForProcess(childPid, killswitch);
		
		//Return if killswitch got set
		if (killswitch) return "";

		//Check if latex was successful
		if (status != EXIT_SUCCESS) {
			stringstream message;
			message << "Error while executing " << arguments[0] << ", it exited with code " << status;
			throw LatexExecutionError(message.str());
		}
	}

	filesystem::path finalPdf = moveResultToOutputDirectory(workingDirectory, outputDirectory);
	return finalPdf;
}
