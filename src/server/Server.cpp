#include "Server.hpp"

  CertificateGeneratorHandler::CertificateGeneratorHandler(const string& id): id(id) {
    // Your initialization goes here
    cout << "Connected " << id << endl;
    
    //Create paths for output and working directory
    try{
	    filesystem::path outputDirectory = baseConfiguration["outputDirectory"].get<std::string>();
	    outputDirectory.append(id);
	    batchConfiguration["outputDirectory"] = outputDirectory.string();
		filesystem::path workingDirectory = baseConfiguration["workingDirectory"].get<std::string>();
		workingDirectory.append(id);
		batchConfiguration["workingDirectory"] = workingDirectory.string();
		
		//Create working directory, if it doesn't exist.
		try{
			filesystem::create_directories(workingDirectory);
		}catch (const std::exception& e){
		    stringstream message;
			message << "Failed to create workingDirectory " << workingDirectory.string();
			throw FileAccessError(message.str());
		}
		
		//Create output directory, if it doesn't exist.
		try{
			filesystem::create_directories(outputDirectory);
		}catch (const std::exception& e){
		    stringstream message;
			message << "Failed to create outputDirectory " << workingDirectory.string();
			throw FileAccessError(message.str());
		}
		
	}catch(const nlohmann::detail::exception& error){
		stringstream message;
		message << "Error while adding base configuration: " << error.what();
		throw InvalidConfigurationError(message.str());
	}
	
	//initialize templates and resources as arrays
	batchConfiguration["templates"] = baseConfiguration["templates"];
	batchConfiguration["resources"] = baseConfiguration["resources"];
  }
  
  CertificateGeneratorHandler::~CertificateGeneratorHandler(){
	  cout << "Disconnected " << id << endl;
  }
    
  void CertificateGeneratorHandler::setConfigurationData(const std::string& configuration) {
    	//Parse received new configuration
    	json newConfiguration;
		try{
			newConfiguration = json::parse(configuration);
		}catch(const nlohmann::detail::parse_error& error){
			stringstream message;
			message << "Invalid json in received batch configuration: " << error.what();
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		
		//Check, that there are no configuration entries set
		if(newConfiguration["outputDirectory"] != nullptr){
			stringstream message;
			message << "Illegal entry outputDirectory in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if(newConfiguration["workingDirectory"] != nullptr){
			stringstream message;
			message << "Illegal entry workingDirectory in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if(newConfiguration["templates"] != nullptr){
			stringstream message;
			message << "Illegal entry templates in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		if(newConfiguration["resources"] != nullptr){
			stringstream message;
			message << "Illegal entry resources in received batch configuration, the server will set this";
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
		
		//Add old configuration
		try{
			newConfiguration["outputDirectory"] = batchConfiguration["outputDirectory"];
			newConfiguration["workingDirectory"] = batchConfiguration["workingDirectory"];
			newConfiguration["templates"] = batchConfiguration["templates"];
			newConfiguration["resources"] = batchConfiguration["resources"];
		}catch(const nlohmann::detail::exception& error){
			stringstream message;
			message << "Error while adding base configuration: " << error.what();
			throw InvalidConfigurationError(message.str());
		}
		
		//Replace old configuration with new configuration
		//TODO thread safty
		batchConfiguration = newConfiguration;
  }
  
  void CertificateGeneratorHandler::addResourceFile(const File& receivedResourceFile) {
	//Sanitize filename
	File resourceFile(receivedResourceFile);
	if( !sanitizeFilename(resourceFile.name) ){
		//Abort if filename is invalid, because it is probably reffered to in the template or other resources
		stringstream message;
		message << "Invalid resource filename." << 
		"Valid filenames must only contain characters from the posix " <<
		"portable filename character set, only contain the filename " << 
		"without a path, be shorter than 255 characters, " <<
		"not start with a hyphen and not be \".\", \"..\" or \"_\"." <<
		"A valid version of your filename would be: " << resourceFile.name;
		InvalidResource terror;
		terror.message = message.str();
		throw terror;
	}
	
	//Save file to disk
    filesystem::path resourcePath(batchConfiguration["workingDirectory"]);
    resourcePath.append( filesystem::path(resourceFile.name).filename().string() );
    ofstream resourceFileStream(resourcePath, ios::out | ios::binary);
    if(!resourceFileStream){
		InternalServerError terror;
		terror.message = "Failed to write resourceFile.";
		throw terror;
	}
	resourceFileStream << resourceFile.content;
	resourceFileStream.close();
	
	//Add to list of resources
	batchConfiguration["resources"].push_back(resourcePath.filename().string());
  }

  void CertificateGeneratorHandler::addTemplateFile(const File& receivedTemplateFile) {
	File templateFile(receivedTemplateFile);
	//Sanitize the filename
	sanitizeFilename(templateFile.name);
	
	//Save file to disk
    filesystem::path templatePath(batchConfiguration["workingDirectory"]);
    templatePath.append( filesystem::path(templateFile.name).filename().string() );
    ofstream templateFileStream(templatePath, ios::out | ios::binary);
    if(!templateFileStream){
		stringstream message;
		message << "Failed to write templateFile.";
		InternalServerError terror;
		terror.message = message.str();
		throw terror;
	}
	templateFileStream << templateFile.content;
	templateFileStream.close();
	
	//Add to list of templates
	batchConfiguration["templates"].push_back(templatePath.filename().string());
  }
  
  void CertificateGeneratorHandler::addResourceFiles(const std::vector<File> & resourceFiles) {
    for(File resourceFile : resourceFiles){
		addResourceFile(resourceFile);
	}
  }

  void CertificateGeneratorHandler::addTemplateFiles(const std::vector<File> & templateFiles) {
    for(File templateFile : templateFiles){
		addTemplateFile(templateFile);
	}
  }

  bool CertificateGeneratorHandler::checkJob() {
		//Create batch
		try{
			Batch batch(batchConfiguration);
			//Check batch
			std::cout << "Checking Batch" << std::endl;
			if(batch.check()){
				std::cout << "Check succeeded" << std::endl;
				return true;
			}else{
				std::cout << "Check failed" << std::endl;
				return false;
			}
		}catch(const InvalidConfigurationError& error){
			stringstream message;
			message << "Invalid configuration: " << error.what();
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
  }

  void CertificateGeneratorHandler::generateCertificates(std::vector<File> & _return) {
		
		try{
			//Create batch
			Batch batch(batchConfiguration);
			
			//Execute batch
			std::cout << "Executing Batch" << std::endl;
			try{
				batch.executeBatch();
			}catch(const InvalidConfigurationError& error){
				stringstream message;
				message << "Error while generating certificates: " << error.what();
				InvalidConfiguration terror;
				terror.message = message.str();
				throw terror;
			}
			std::cout << "All done" << std::endl;
			
			//Returning results
			std::cout << "Returning result" << std::endl;
			vector<File> generatedFiles;
			for(string outputFile : batch.getOutputFiles() ){
				File file;
				file.name = filesystem::path(outputFile).filename();
				stringstream content;
				ifstream pdfFile(outputFile, ios::in | ios::binary);
				if(!pdfFile){
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
			std::cout << "Cleaning files" << std::endl;
			filesystem::remove_all(batchConfiguration["workingDirectory"].get<std::string>());
			if(!keepGeneratedFiles){
				filesystem::remove_all(batchConfiguration["outputDirectory"].get<std::string>());
			}
		}catch(const InvalidConfigurationError& error){
			stringstream message;
			message << "Invalid configuration: " << error.what();
			InvalidConfiguration terror;
			terror.message = message.str();
			throw terror;
		}
  }
  
  bool CertificateGeneratorHandler::sanitizeFilename(string& filename){
	bool validName = true;
	
	//Strip path away, if present
	try{
		filesystem::path filePath(filename);
		string onlyFilename = filePath.filename().string();
		if(filename != onlyFilename){
			validName = false;
		}
		filename = onlyFilename;
	}catch(const exception& e){
		validName = false;
	}
	
	//Only allow 0-9 a-z A-Z - _  and .
	string cleanName;
    for(char c: filename){
		if( (c>='0'&&c<='9') || (c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c=='-') || (c=='_') || (c=='.') ){
			cleanName.push_back(c);
		}else{
			validName = false;
		}
	}
	filename = cleanName;
	
	//Check that filename longer than 254 characters
	if(filename.size() >= 255){
		validName = false;
		filename = filename.substr(0,254);
	}
	
	//Check that filename probably wont be interpreted as an option
	while( filename.size()>0 && filename[0] == '-' ){
		validName = false;
		filename = filename.substr(1,string::npos);
	}
	
	//Check that filename is valid and if not generate new one
	if(filename.size() == 0 || filename == ".." || filename == "." || filename == "_"){
		validName = false;
		string randomName;
		randomName.reserve(8);
		for(int i = 0; i<8 ; i++){
			randomName.push_back( (unsigned char)((rand()%26)+65) );
		}
		filename = randomName;
	}
	
	return validName;
  }

  CertificateGeneratorIf* CertificateGeneratorCloneFactory::getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    std::stringstream id;
    //id << (now->tm_year%100) << "-" << std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << "-" << std::setfill('0') << std::setw(2) << (now->tm_hour) << "-" << std::setfill('0') << std::setw(2) << (now->tm_min) << "-" << std::setfill('0') << std::setw(2) << (now->tm_sec) << "_" << sock->getPeerAddress() << "_" << count++ ;
    id << (now->tm_year%100) << "-" << std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << "-" << std::setfill('0') << std::setw(2) << (now->tm_hour) << "-" << std::setfill('0') << std::setw(2) << (now->tm_min) << "-" << std::setfill('0') << std::setw(2) << (now->tm_sec) << "_" << count++ ;
    return new CertificateGeneratorHandler(id.str());
  }
  
  void CertificateGeneratorCloneFactory::releaseHandler( CertificateGeneratorIf* handler) {
    delete handler;
  }
  

int main(int argc, char **argv) {

	//Parse options
	string batchConfigurationFile;
	string workingDirectory;
	string outputDirectory;
	int serverPort;
	bool verbose = false;
	
	bool docker;
	bool useThreads;
	int maxWorkersPerBatch;
	int maxMemoryPerWorker;
	int maxCpuTimePerWorker;
	int workerTimeout;
	int batchTimeout;
	
	try{
		cxxopts::Options options(argv[0], "Certificate generator server");
		options.add_options()
			("c,configuration", "The base configuration file", cxxopts::value<string>(), "FILE")
			//("w,working-dir", "The working directory", cxxopts::value<string>(), "PATH")
			//("o,output-dir", "The output directory", cxxopts::value<string>(), "PATH")
			("p,port", "The port on which the server listens", cxxopts::value<int>())
			("k,keep-files", "Keep generated files", cxxopts::value<bool>(keepGeneratedFiles))
			("v,verbose", "Enable output", cxxopts::value<bool>(verbose))
			("help", "Print help");
		options.add_options("Resource managment")
			("use-docker", "Each compiler process runs in its own docker container", cxxopts::value<bool>(docker)->default_value(MTOS(DEFAULT_DOCKER))->implicit_value("true"))
			("use-threads", "Multiple compiler processes/containers run in parallel", cxxopts::value<bool>(useThreads)->default_value(MTOS(DEFAULT_USE_THREAD))->implicit_value("true"))
			("max-batch-threads", "Maximum number of parallel compiler processes/containers per batch", cxxopts::value<int>(maxWorkersPerBatch)->default_value(MTOS(DEFAULT_MAX_WORKERS)), "INT")
			("max-compiler-memory", "Maximum memory per compiler process/container", cxxopts::value<int>(maxMemoryPerWorker)->default_value(MTOS(DEFAULT_MAX_MEMORY)), "BYTES")
			("max-compiler-cpu-time", "Maximum cpu time per compiler process, ignored if --use-docker is set", cxxopts::value<int>(maxCpuTimePerWorker)->default_value(MTOS(DEFAULT_MAX_CPU)), "SECONDS")
			("compiler-timeout", "Timeout after which compiler processes/containers are killed", cxxopts::value<int>(workerTimeout)->default_value(MTOS(DEFAULT_WORKER_TIMEOUT)), "SECONDS")
			("batch-timeout", "Timeout after which a batch is killed, not implemented yet", cxxopts::value<int>(batchTimeout)->default_value(MTOS(DEFAULT_TIMEOUT)), "SECONDS");
		auto result = options.parse(argc, argv);
		if (result.count("help") || result.arguments().size()==0){
			cout << options.help({"", "Resource managment"}) << std::endl;
			exit(EXIT_SUCCESS);
		}
		if (result.count("configuration")){
			batchConfigurationFile = result["configuration"].as<string>();
		}else{
			throw cxxopts::OptionException("No base configuration file specified");
		}
		if (result.count("port")){
			serverPort = result["port"].as<int>();
			if(serverPort > 65535 || serverPort < 1){
				throw cxxopts::OptionException("Invalid port specified");
			}
		}else{
			throw cxxopts::OptionException("No port specified");
		}
		if (result.count("max-batch-compilers") && maxWorkersPerBatch <= 0){
			throw cxxopts::OptionException("Invalid number of parallel compiler processes/containers specified");
		}
		if (result.count("max-compiler-memory") && maxMemoryPerWorker <= 0){
			throw cxxopts::OptionException("Invalid maximum memory per compiler process/container specified");
		}
		if (result.count("max-compiler-cpu-time") && maxCpuTimePerWorker <= 0){
			throw cxxopts::OptionException("Invalid maximum cpu time per compiler process specified");
		}
		if (result.count("compiler-timeout") && workerTimeout <= 0){
			throw cxxopts::OptionException("Invalid timeout per compiler process/container specified");
		}
		if (result.count("batch-timeout") && batchTimeout <= 0){
			throw cxxopts::OptionException("Invalid timeout per batch specified");
		}
	}catch (const cxxopts::OptionException& e){
		cerr << "Error parsing options: " << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	
	//Set configuration
	Configuration::setup(docker, useThreads, maxWorkersPerBatch, maxMemoryPerWorker, maxCpuTimePerWorker, workerTimeout, batchTimeout);
	
	//Disable cout
	if(!verbose){
		cout.rdbuf(NULL);
	}
	
	//Load batch configuration
	std::cout << "Initializing server" << std::endl;
	std::cout << "Loading base configuration" << std::endl;
	ifstream input;
	input.open(batchConfigurationFile, ios::in);
	if(!input){
		cerr << "Error reading base configuration file" << endl;
		exit(EXIT_FAILURE);
	}
	try{
		baseConfiguration = json::parse(input);
	}catch(const nlohmann::detail::parse_error& error){
		cerr << "Invalid json in base configuration: " << batchConfigurationFile << endl;
		cerr << error.what() << endl;
		exit(EXIT_FAILURE);
	}
	input.close();
	
	//Check base configuration
	std::cout << "Checking base configuration" << std::endl;
	bool abort = false;
	if(!baseConfiguration["outputDirectory"].is_string()){
		std::cerr << "Error checking configuration: No output directory specified" << std::endl;
		abort = true;
	}
	if(!baseConfiguration["workingDirectory"].is_string()){
		std::cerr << "Error checking configuration: No working directory specified" << std::endl;
		abort = true;
	}
	if(abort){
		exit(EXIT_FAILURE);
	}
	
	//Initialize thrift server
	int port = serverPort;
	::std::shared_ptr<CertificateGeneratorProcessorFactory> processorFactory(std::make_shared<CertificateGeneratorProcessorFactory>(std::make_shared<CertificateGeneratorCloneFactory>()));
	::std::shared_ptr<TServerTransport> serverTransport(std::make_shared<TServerSocket>(port));
	::std::shared_ptr<TTransportFactory> transportFactory(std::make_shared<TBufferedTransportFactory>());
	::std::shared_ptr<TProtocolFactory> protocolFactory(std::make_shared<TBinaryProtocolFactory>());

	//Open thrift server
	std::cout << "Starting server" << std::endl;
	TThreadedServer server(processorFactory, serverTransport, transportFactory, protocolFactory);
	server.serve();
	return 0;
}
