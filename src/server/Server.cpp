#include "Server.hpp"

CertificateGeneratorHandler::CertificateGeneratorHandler(const string& id, const string& peerAddress)
	: id(id)
	, peerAddress(peerAddress)
{
	try {

		spdlog::info("{} connected (ID:{})", peerAddress, id);

		//Create paths for output and working directory
		try {
			filesystem::path outputDirectory = baseConfiguration["outputDirectory"].get<std::string>();
			outputDirectory.append(id);
			batchConfiguration["outputDirectory"] = outputDirectory.string();
			filesystem::path workingDirectory = baseConfiguration["workingDirectory"].get<std::string>();
			workingDirectory.append(id);
			batchConfiguration["workingDirectory"] = workingDirectory.string();

			//Create working directory, if it doesn't exist.
			try {
				filesystem::create_directories(workingDirectory);
			} catch (const std::exception& e) {
				stringstream message;
				message << "Failed to create workingDirectory " << workingDirectory.string();
				throw FileAccessError(message.str());
			}

			//Create output directory, if it doesn't exist.
			try {
				filesystem::create_directories(outputDirectory);
			} catch (const std::exception& e) {
				stringstream message;
				message << "Failed to create outputDirectory " << workingDirectory.string();
				throw FileAccessError(message.str());
			}

			//initialize templates and resources as arrays
			batchConfiguration["templates"] = baseConfiguration["templates"];
			batchConfiguration["resources"] = baseConfiguration["resources"];
		} catch (const nlohmann::detail::exception& error) {
			stringstream message;
			message << "Error while adding base configuration: " << error.what();
			throw InvalidConfigurationError(message.str());
		}
	} catch (const GeneratorError& error) {
		if (dontCrash) {
			spdlog::error("{} failed in constructor (ID:{}) GeneratorError ignored, because of --dont-crash: {}", peerAddress, id, error.what());
		} else {
			spdlog::critical("{} failed in constructor (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		}
	} catch (const TException& error) {
		if (dontCrash) {
			spdlog::error("{} failed in constructor (ID:{}) ThriftException ignored, because of --dont-crash: {}", peerAddress, id, error.what());
		} else {
			spdlog::critical("{} failed in constructor (ID:{}) ThriftException: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in constructor (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
		} else {
			spdlog::critical("{} failed in constructor (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in constructor (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
		} else {
			spdlog::critical("{} failed in constructor (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

CertificateGeneratorHandler::~CertificateGeneratorHandler()
{
	if (!keepGeneratedFiles) {
		try{
			filesystem::remove_all(batchConfiguration["workingDirectory"].get<std::string>());
			filesystem::remove_all(batchConfiguration["outputDirectory"].get<std::string>());
		}catch(...){
			spdlog::info("{} error removing files on destructor (ID:{})", peerAddress, id);
		}
	}
	spdlog::info("{} disconnected (ID:{})", peerAddress, id);
}

void CertificateGeneratorHandler::setConfigurationData(const std::string& configuration)
{
	spdlog::info("{} called setConfigurationData (ID:{})", peerAddress, id);
	try {
		//Parse received new configuration
		json newConfiguration;
		try {
			newConfiguration = json::parse(configuration);
		} catch (const nlohmann::detail::parse_error& error) {
			stringstream message;
			message << "Invalid json in received batch configuration: " << error.what();
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}

		//Check, that there are no configuration entries set
		if (newConfiguration["outputDirectory"] != nullptr) {
			stringstream message;
			message << "Illegal entry outputDirectory in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if (newConfiguration["workingDirectory"] != nullptr) {
			stringstream message;
			message << "Illegal entry workingDirectory in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if (newConfiguration["templates"] != nullptr) {
			stringstream message;
			message << "Illegal entry templates in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if (newConfiguration["resources"] != nullptr) {
			stringstream message;
			message << "Illegal entry resources in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}

		//Add old configuration
		try {
			newConfiguration["outputDirectory"] = batchConfiguration["outputDirectory"];
			newConfiguration["workingDirectory"] = batchConfiguration["workingDirectory"];
			newConfiguration["templates"] = batchConfiguration["templates"];
			newConfiguration["resources"] = batchConfiguration["resources"];
		} catch (const nlohmann::detail::exception& error) {
			stringstream message;
			message << "Error while adding base configuration: " << error.what();
			throw InvalidConfigurationError(message.str());
		}

		//Replace old configuration with new configuration
		//TODO thread safty
		batchConfiguration = newConfiguration;
	} catch (const GeneratorError& error) {
		spdlog::warn("{} failed in setConfigurationData (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
		InternalServerError terror;
		terror.message = "Internal server error, try again later.";
		throw terror;
	} catch (const TException& error) {
		spdlog::warn("{} failed in setConfigurationData (ID:{}) ThriftException: {}", peerAddress, id, error.what());
		throw;
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in setConfigurationData (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in setConfigurationData (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in setConfigurationData (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in setConfigurationData (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

void CertificateGeneratorHandler::addResourceFile(const File& receivedResourceFile)
{
	spdlog::info("{} called addResourceFile (ID:{})", peerAddress, id);
	try {
		//Sanitize filename
		File resourceFile(receivedResourceFile);
		if (!sanitizeFilename(resourceFile.name)) {
			//Abort if filename is invalid, because it is probably reffered to in the template or other resources
			stringstream message;
			message << "Invalid resource filename."
					<< "Valid filenames must only contain characters from the posix "
					<< "portable filename character set, only contain the filename "
					<< "without a path, be shorter than 255 characters, "
					<< "not start with a hyphen and not be \".\", \"..\" or \"_\"."
					<< "A valid version of your filename would be: " << resourceFile.name;
			InvalidResource terror;
			terror.message = message.str();
			throw terror;
		}

		//Save file to disk
		filesystem::path resourcePath(batchConfiguration["workingDirectory"]);
		resourcePath.append(filesystem::path(resourceFile.name).filename().string());
		ofstream resourceFileStream(resourcePath, ios::out | ios::binary);
		if (!resourceFileStream) {
			InternalServerError terror;
			terror.message = "Failed to write resourceFile.";
			throw terror;
		}
		resourceFileStream << resourceFile.content;
		resourceFileStream.close();

		//Add to list of resources
		json temp = batchConfiguration["resources"];
		if(find(temp.begin(), temp.end(), resourcePath.filename().string() ) == temp.end()){
			batchConfiguration["resources"].push_back(resourcePath.filename().string());
		}
	} catch (const GeneratorError& error) {
		spdlog::warn("{} failed in addResourceFile (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
		InternalServerError terror;
		terror.message = "Internal server error, try again later.";
		throw terror;
	} catch (const TException& error) {
		spdlog::warn("{} failed in addResourceFile (ID:{}) ThriftException: {}", peerAddress, id, error.what());
		throw;
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in addResourceFile (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in addResourceFile (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in addResourceFile (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in addResourceFile (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

void CertificateGeneratorHandler::addTemplateFile(const File& receivedTemplateFile)
{
	spdlog::info("{} called addTemplateFile (ID:{})", peerAddress, id);
	try {
		File templateFile(receivedTemplateFile);
		//Sanitize the filename
		sanitizeFilename(templateFile.name);

		//Save file to disk
		filesystem::path templatePath(batchConfiguration["workingDirectory"]);
		templatePath.append(filesystem::path(templateFile.name).filename().string());
		ofstream templateFileStream(templatePath, ios::out | ios::binary);
		if (!templateFileStream) {
			stringstream message;
			message << "Failed to write templateFile.";
			InternalServerError terror;
			terror.message = message.str();
			throw terror;
		}
		templateFileStream << templateFile.content;
		templateFileStream.close();

		//Add to list of templates
		json temp = batchConfiguration["templates"];
		if(find(temp.begin(), temp.end(), templatePath.filename().string() ) == temp.end()){
			batchConfiguration["templates"].push_back(templatePath.filename().string());
		}
	} catch (const GeneratorError& error) {
		spdlog::warn("{} failed in addTemplateFile (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
		InternalServerError terror;
		terror.message = "Internal server error, try again later.";
		throw terror;
	} catch (const TException& error) {
		spdlog::warn("{} failed in addTemplateFile (ID:{}) ThriftException: {}", peerAddress, id, error.what());
		throw;
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in addTemplateFile (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in addTemplateFile (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in addTemplateFile (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in addTemplateFile (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

void CertificateGeneratorHandler::addResourceFiles(const std::vector<File>& resourceFiles)
{
	for (File resourceFile : resourceFiles) {
		addResourceFile(resourceFile);
	}
}

void CertificateGeneratorHandler::addTemplateFiles(const std::vector<File>& templateFiles)
{
	for (File templateFile : templateFiles) {
		addTemplateFile(templateFile);
	}
}

bool CertificateGeneratorHandler::checkJob()
{
	spdlog::info("{} called checkJob (ID:{})", peerAddress, id);
	//Create batch
	try {
		Batch batch(batchConfiguration);
		//Check batch
		if (batch.check()) {
			spdlog::debug("{} Check succeeded (ID:{})", peerAddress, id);
			return true;
		} else {
			spdlog::debug("{} Check failed (ID:{})", peerAddress, id);
			return false;
		}
	} catch (const InvalidConfigurationError& error) {
		spdlog::warn("{} failed in checkJob (ID:{}) InvalidConfigurationError: {}", peerAddress, id, error.what());
		stringstream message;
		spdlog::debug("Invalid configuration: {}", error.what());
		InvalidConfiguration terror;
		terror.message = message.str();
		throw terror;
	} catch (const InvalidTemplateError& error) {
		spdlog::warn("{} failed in checkJob (ID:{}) InvalidTemplateError: {}", peerAddress, id, error.what());
		stringstream message;
		spdlog::debug("Invalid template: ", error.what());
		InvalidTemplate terror;
		terror.message = message.str();
		throw terror;
	} catch (const GeneratorError& error) {
		spdlog::warn("{} failed in checkJob (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
		InternalServerError terror;
		terror.message = "Internal server error, try again later.";
		throw terror;
	} catch (const TException& error) {
		spdlog::warn("{} failed in checkJob (ID:{}) ThriftException: {}", peerAddress, id, error.what());
		throw;
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in checkJob (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in checkJob (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in checkJob (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in checkJob (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

void CertificateGeneratorHandler::generateCertificates(std::vector<File>& _return)
{
	spdlog::info("{} called generateCertificates (ID:{})", peerAddress, id);
	try {
		//Create batch
		Batch batch(batchConfiguration);

		//Execute batch
		spdlog::trace("{} executing batch (ID:{})", peerAddress, id);
		try {
			batch.executeBatch();
		} catch (const InvalidConfigurationError& error) {
			stringstream message;
			spdlog::trace("{} failed while generating certificates (ID:{})", peerAddress, id);
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		spdlog::trace("{} generation done (ID:{})", peerAddress, id);

		//Returning results
		spdlog::trace("{} returning results (ID:{})", peerAddress, id);
		vector<File> generatedFiles;
		for (string outputFile : batch.getOutputFiles()) {
			File file;
			file.name = filesystem::path(outputFile).filename();
			stringstream content;
			ifstream pdfFile(outputFile, ios::in | ios::binary);
			if (!pdfFile) {
				InvalidConfiguration thriftError;
				thriftError.message = "Failed to open output file";
				throw thriftError;
			}
			//TODO pdfFile >> file.content;  ?
			content << pdfFile.rdbuf();
			pdfFile.close();
			file.content = content.str();
			generatedFiles.push_back(file);
		}
		_return = generatedFiles;

		//Removing files
		spdlog::trace("{} Cleaning files (ID:{})", peerAddress, id);
		if (!keepGeneratedFiles) {
			filesystem::remove_all(batchConfiguration["outputDirectory"].get<std::string>());
		}
	} catch (const InvalidConfigurationError& error) {
		spdlog::warn("{} failed in generateCertificates (ID:{}) InvalidConfigurationError: {}", peerAddress, id, error.what());
		stringstream message;
		spdlog::debug("Invalid configuration: {}", error.what());
		InvalidConfiguration terror;
		terror.message = message.str();
		throw terror;
	} catch (const InvalidTemplateError& error) {
		spdlog::warn("{} failed in generateCertificates (ID:{}) InvalidTemplateError: {}", peerAddress, id, error.what());
		stringstream message;
		spdlog::debug("Invalid template: ", error.what());
		InvalidTemplate terror;
		terror.message = message.str();
		throw terror;
	} catch (const GeneratorError& error) {
		spdlog::warn("{} failed in generateCertificates (ID:{}) GeneratorError: {}", peerAddress, id, error.what());
		InternalServerError terror;
		terror.message = "Internal server error, try again later.";
		throw terror;
	} catch (const TException& error) {
		spdlog::warn("{} failed in generateCertificates (ID:{}) ThriftException: {}", peerAddress, id, error.what());
		throw;
	} catch (const exception& error) {
		if (dontCrash) {
			spdlog::error("{} failed in generateCertificates (ID:{}) Unhandled exception ignored, because of --dont-crash: {}", peerAddress, id, error.what());
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in generateCertificates (ID:{}) Unhandled exception: {}", peerAddress, id, error.what());
			throw;
		}
	} catch (...) {
		if (dontCrash) {
			spdlog::error("{} failed in generateCertificates (ID:{}) Unhandled error ignored, because of --dont-crash", peerAddress, id);
			InternalServerError terror;
			terror.message = "Internal server error, try again later.";
			throw terror;
		} else {
			spdlog::critical("{} failed in generateCertificates (ID:{}) Unhandled error", peerAddress, id);
			throw;
		}
	}
}

bool CertificateGeneratorHandler::sanitizeFilename(string& filename)
{
	bool validName = true;

	//Strip path away, if present
	try {
		filesystem::path filePath(filename);
		string onlyFilename = filePath.filename().string();
		if (filename != onlyFilename) {
			validName = false;
		}
		filename = onlyFilename;
	} catch (const exception& e) {
		validName = false;
	}

	//Only allow 0-9 a-z A-Z - _  and .
	string cleanName;
	for (char c : filename) {
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '-') || (c == '_') || (c == '.')) {
			cleanName.push_back(c);
		} else {
			validName = false;
		}
	}
	filename = cleanName;

	//Check that filename longer than 254 characters
	if (filename.size() >= 255) {
		validName = false;
		filename = filename.substr(0, 254);
	}

	//Check that filename probably wont be interpreted as an option
	while (filename.size() > 0 && filename[0] == '-') {
		validName = false;
		filename = filename.substr(1, string::npos);
	}

	//Check that filename is valid and if not generate new one
	if (filename.size() == 0 || filename == ".." || filename == "." || filename == "_") {
		validName = false;
		string randomName;
		randomName.reserve(8);
		for (int i = 0; i < 8; i++) {
			randomName.push_back((unsigned char)((rand() % 26) + 65));
		}
		filename = randomName;
	}

	return validName;
}

CertificateGeneratorIf* CertificateGeneratorCloneFactory::getHandler(const ::apache::thrift::TConnectionInfo& connInfo)
{
	std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	std::stringstream id;
	id << (now->tm_year % 100) << "-" << std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << "-" << std::setfill('0') << std::setw(2) << (now->tm_hour) << "-" << std::setfill('0') << std::setw(2) << (now->tm_min) << "-" << std::setfill('0') << std::setw(2) << (now->tm_sec) << "_" << count++;
	return new CertificateGeneratorHandler(id.str(), sock->getPeerAddress());
}

void CertificateGeneratorCloneFactory::releaseHandler(CertificateGeneratorIf* handler)
{
	delete handler;
}

int main(int argc, char** argv)
{

	string batchConfigurationFile;
	string workingDirectory;
	string outputDirectory;
	int serverPort;

	bool docker;
	bool useThreads;
	int maxWorkersPerBatch;
	int maxMemoryPerWorker;
	int maxCpuTimePerWorker;
	int workerTimeout;
	int batchTimeout;
	int maxWorkers;

	spdlog::level::level_enum logLevel = spdlog::level::info;
	spdlog::level::level_enum logfileLevel = spdlog::level::info;
	string logfileDirectory;

	//Parse options
	spdlog::debug("Parsing options");
	try {
		cxxopts::Options options(argv[0], "Certificate generator server");
		options.add_options()("c,configuration", "The base configuration file", cxxopts::value<string>(), "FILE")
			//("w,working-dir", "The working directory", cxxopts::value<string>(), "PATH")
			//("o,output-dir", "The output directory", cxxopts::value<string>(), "PATH")
			("p,port", "The port on which the server listens", cxxopts::value<int>())("k,keep-files", "Keep generated files", cxxopts::value<bool>(keepGeneratedFiles))("dont-crash", "Catch all exceptions inside handlers", cxxopts::value<bool>(dontCrash))("help", "Print help");
		options.add_options("Resource managment")("use-docker", "Each compiler process runs in its own docker container", cxxopts::value<bool>(docker)->default_value(MTOS(DEFAULT_DOCKER))->implicit_value("true"))("use-threads", "Multiple compiler processes/containers run in parallel", cxxopts::value<bool>(useThreads)->default_value(MTOS(DEFAULT_USE_THREAD))->implicit_value("true"))("max-batch-compilers", "Maximum number of parallel compiler processes/containers per batch", cxxopts::value<int>(maxWorkersPerBatch)->default_value(MTOS(DEFAULT_MAX_BATCH_WORKERS)), "INT")("max-compilers", "Maximum number of parallel compiler processes/containers", cxxopts::value<int>(maxWorkers)->default_value(MTOS(DEFAULT_MAX_WORKERS)), "INT")("max-compiler-memory", "Maximum memory per compiler process/container", cxxopts::value<int>(maxMemoryPerWorker)->default_value(MTOS(DEFAULT_MAX_MEMORY)), "BYTES")("max-compiler-cpu-time", "Maximum cpu time per compiler process, ignored if --use-docker is set", cxxopts::value<int>(maxCpuTimePerWorker)->default_value(MTOS(DEFAULT_MAX_CPU)), "SECONDS")("compiler-timeout", "Timeout after which compiler processes/containers are killed", cxxopts::value<int>(workerTimeout)->default_value(MTOS(DEFAULT_WORKER_TIMEOUT)), "SECONDS")("batch-timeout", "Timeout after which a batch is killed, not implemented yet", cxxopts::value<int>(batchTimeout)->default_value(MTOS(DEFAULT_TIMEOUT)), "SECONDS");
		options.add_options("Logging")("d,debug", "Output information, errors and debug messages", cxxopts::value<bool>())("i,info", "Output information and errors", cxxopts::value<bool>()->default_value("true"))("e,error", "Output only errors", cxxopts::value<bool>())("q,quiet", "Output nothing", cxxopts::value<bool>())("log-directory", "Write logfiles into this directory", cxxopts::value<string>(logfileDirectory), "DIR")("log-debug", "Output debug messages, information and errors to logfiles", cxxopts::value<bool>())("log-info", "Output information and errors", cxxopts::value<bool>()->default_value("true"))("log-error", "Output only errors", cxxopts::value<bool>())("log-quiet", "Output nothing", cxxopts::value<bool>());
		auto result = options.parse(argc, argv);
		if (result.count("help") || result.arguments().size() == 0) {
			cout << options.help({ "", "Resource managment", "Logging" }) << std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("configuration")) {
			batchConfigurationFile = result["configuration"].as<string>();
		} else {
			throw cxxopts::OptionException("No base configuration file specified");
		}
		if (result.count("port")) {
			serverPort = result["port"].as<int>();
			if (serverPort > 65535 || serverPort < 1) {
				throw cxxopts::OptionException("Invalid port specified");
			}
		} else {
			throw cxxopts::OptionException("No port specified");
		}
		if (result.count("max-batch-compilers") && maxWorkersPerBatch <= 0) {
			throw cxxopts::OptionException("Invalid number of parallel compiler processes/containers per batch specified");
		}
		if (result.count("max-compiler-memory") && maxMemoryPerWorker <= 0) {
			throw cxxopts::OptionException("Invalid maximum memory per compiler process/container specified");
		}
		if (result.count("max-compiler-cpu-time") && maxCpuTimePerWorker <= 0) {
			throw cxxopts::OptionException("Invalid maximum cpu time per compiler process specified");
		}
		if (result.count("compiler-timeout") && workerTimeout < 0) {
			throw cxxopts::OptionException("Invalid timeout per compiler process/container specified");
		}
		if (result.count("batch-timeout") && batchTimeout < 0) {
			throw cxxopts::OptionException("Invalid timeout per batch specified");
		}
		if (result.count("max-compilers") && maxWorkers <= 0) {
			throw cxxopts::OptionException("Invalid number of parallel compiler processes/containers specified");
		}
		if (result.count("quiet") && result["quiet"].as<bool>()) {
			logLevel = spdlog::level::off;
		}
		if (result.count("error") && result["error"].as<bool>()) {
			logLevel = spdlog::level::err;
		}
		if (result.count("info") && result["info"].as<bool>()) {
			logLevel = spdlog::level::info;
		}
		if (result.count("debug") && result["debug"].as<bool>()) {
			logLevel = spdlog::level::trace;
		}
		if (result.count("log-quiet") && result["log-quiet"].as<bool>()) {
			logfileLevel = spdlog::level::off;
		}
		if (result.count("log-error") && result["log-error"].as<bool>()) {
			logfileLevel = spdlog::level::err;
		}
		if (result.count("log-info") && result["log-info"].as<bool>()) {
			logfileLevel = spdlog::level::info;
		}
		if (result.count("log-debug") && result["log-debug"].as<bool>()) {
			logfileLevel = spdlog::level::trace;
		}
	} catch (const cxxopts::OptionException& e) {
		spdlog::critical("Error parsing options: {}", e.what());
		exit(EXIT_FAILURE);
	}

	//Set logger
	spdlog::sink_ptr color = make_shared<spdlog::sinks::stdout_color_sink_mt>();
	color->set_level(logLevel);
	spdlog::init_thread_pool(8192, 1);
	auto logger = std::make_shared<spdlog::async_logger>("", color, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	spdlog::drop("");
	spdlog::register_logger(logger);
	spdlog::set_default_logger(logger);
	logger->flush_on(spdlog::level::warn);
	spdlog::flush_every(std::chrono::minutes(1));
	logger->set_level(spdlog::level::trace);
	if (logfileDirectory != "") {
		try {
			filesystem::path baseLogfile(logfileDirectory);
			filesystem::create_directories(baseLogfile);
			baseLogfile.append("logfile");
			spdlog::sink_ptr dailyfile = std::make_shared<spdlog::sinks::daily_file_sink_mt>(baseLogfile.string(), 17, 54);
			dailyfile->set_level(logfileLevel);
			logger->sinks().push_back(dailyfile);
		} catch (const filesystem::filesystem_error& error) {
			spdlog::critical("Cannot access log directory");
			exit(EXIT_FAILURE);
		}
	}
	spdlog::debug("Logger initialized");

	//Set configuration
	spdlog::debug("Setting configuration");
	Configuration::setup(docker, useThreads, maxWorkersPerBatch, maxMemoryPerWorker, maxCpuTimePerWorker, workerTimeout, batchTimeout, maxWorkers);

	//Load batch configuration
	spdlog::debug("Loading base configuration");
	ifstream input;
	input.open(batchConfigurationFile, ios::in);
	if (!input) {
		spdlog::critical("Error reading base configuration file");
		exit(EXIT_FAILURE);
	}
	try {
		baseConfiguration = json::parse(input);
	} catch (const nlohmann::detail::parse_error& e) {
		spdlog::critical("Invalid json in base configuration {} {}", batchConfigurationFile, e.what());
		exit(EXIT_FAILURE);
	}
	input.close();

	//Check base configuration
	spdlog::debug("Checking base configuration");
	bool abort = false;
	if (!baseConfiguration["outputDirectory"].is_string()) {
		spdlog::critical("Error checking configuration: No output directory specified");
		abort = true;
	}
	if (!baseConfiguration["workingDirectory"].is_string()) {
		spdlog::critical("Error checking configuration: No working directory specified");
		abort = true;
	}
	if (abort) {
		exit(EXIT_FAILURE);
	}

	//Initialize thrift server
	int port = serverPort;
	::std::shared_ptr<CertificateGeneratorProcessorFactory> processorFactory(std::make_shared<CertificateGeneratorProcessorFactory>(std::make_shared<CertificateGeneratorCloneFactory>()));
	::std::shared_ptr<TServerTransport> serverTransport(std::make_shared<TServerSocket>(port));
	::std::shared_ptr<TTransportFactory> transportFactory(std::make_shared<TBufferedTransportFactory>());
	::std::shared_ptr<TProtocolFactory> protocolFactory(std::make_shared<TBinaryProtocolFactory>());

	//Open thrift server
	spdlog::info("Starting server");
	TThreadedServer server(processorFactory, serverTransport, transportFactory, protocolFactory);
	server.serve();
	return 0;
}
