#include "Batch.hpp"

sem_t Batch::globalRemainingWorkplaces;
atomic_char Batch::globalRemainingWorkplacesInitialized = false;

Batch::Batch(vector<Student> students, vector<TemplateCertificate> templateCertificates, const string& workingDirectory, const string& outputDirectory)
	: students(students)
	, templateCertificates(templateCertificates)
	, workingDirectory(workingDirectory)
	, outputDirectory(outputDirectory)
{
}

bool Batch::check() const
{
	//Check every student with every template
	for (TemplateCertificate templateCertificate : templateCertificates) {
		for (Student student : students) {
			if (!templateCertificate.checkStudent(student)) {
				return false;
			}
		}
	}
	return true;
}

void Batch::generateCertificates()
{
	for (TemplateCertificate templateCertificate : templateCertificates) {
		for (Student student : students) {
			certificates.push_back(templateCertificate.generateCertificate(student));
		}
	}
}

void Batch::outputCertificates()
{
	outputFiles.clear();
	if (CONFIG.useThreads) {
		atomic_bool killswitch = false;
		exception_ptr failedThreadException;
		mutex failedThreadExceptionMutex;
		sem_t remainingWorkplaces;
		sem_init(&remainingWorkplaces, 0, CONFIG.maxWorkersPerBatch);
		vector<thread> threads;
		mutex outputFilesMutex;
		for (Certificate certificate : certificates) {
			threads.emplace_back([=, &outputFilesMutex, &remainingWorkplaces, &failedThreadException, &failedThreadExceptionMutex, &killswitch]() {
				try {
					sem_wait(&remainingWorkplaces);
					sem_wait(&globalRemainingWorkplaces);
					if (!killswitch) {
						string generatedPDF = certificate.generatePDF(workingDirectory, outputDirectory, killswitch);
						if (!killswitch) {
							unique_lock<mutex> lock(outputFilesMutex);
							outputFiles.push_back(generatedPDF);
							lock.unlock();
						}
					}
					sem_post(&globalRemainingWorkplaces);
					sem_post(&remainingWorkplaces);
				} catch (...) {
					killswitch = true;
					sem_post(&globalRemainingWorkplaces);
					sem_post(&remainingWorkplaces);
					unique_lock<mutex> lock(failedThreadExceptionMutex);
					if (!failedThreadException) {
						failedThreadException = std::current_exception();
					}
					lock.unlock();
				}
			});
		}
		for (thread& t : threads) {
			t.join();
		}
		unique_lock<mutex> lock(failedThreadExceptionMutex);
		if (failedThreadException) {
			rethrow_exception(failedThreadException);
		}
	} else {
		for (Certificate certificate : certificates) {
			atomic_bool killswitch = false;
			string generatedPDF = certificate.generatePDF(workingDirectory, outputDirectory, killswitch);
			outputFiles.push_back(generatedPDF);
		}
	}
}

void Batch::executeBatch()
{
	generateCertificates();
	outputCertificates();
}

Batch::Batch(json batchConfiguration)
{
	if (!globalRemainingWorkplacesInitialized.fetch_or(true)) {
		spdlog::trace("Initialized global workers");
		sem_init(&globalRemainingWorkplaces, 0, CONFIG.maxWorkers);
	}
	try {
		//Load students
		spdlog::trace("Loading Students");
		for (json person : batchConfiguration["students"]) {
			cout << person["name"] << " " << person["surname"] << endl;
			students.push_back(Student(person));
		}
		//Load templates
		spdlog::trace("Loading Templates");
		for (string templateFile : batchConfiguration["templates"]) {
			filesystem::path templateFilePath(templateFile);
			if (templateFilePath.is_relative()) {
				templateFilePath = batchConfiguration["workingDirectory"].get<string>();
				templateFilePath.append(templateFile);
			}
			spdlog::trace("Loading template file {}", templateFilePath.string());
			ifstream input;
			input.open(templateFilePath, ios::in);
			if (!input) {
				stringstream message;
				message << "Error reading template file " << templateFile;
				throw FileAccessError(message.str());
			}
			std::string templateCertificateContent((std::istreambuf_iterator<char>(input)),
				(std::istreambuf_iterator<char>()));
			input.close();

			//Generate base file name
			string basename = templateFilePath.stem();

			//TODO Maybe not push everything as global
			templateCertificates.push_back(TemplateCertificate(basename, templateCertificateContent, batchConfiguration));
		}

		//Load directories
		outputDirectory = batchConfiguration["outputDirectory"];
		workingDirectory = batchConfiguration["workingDirectory"];
		outputDirectory.append("/");
		workingDirectory.append("/");
		try {
			filesystem::create_directories(workingDirectory);
			filesystem::create_directories(outputDirectory);
		} catch (const std::exception& e) {
			stringstream message;
			message << "Failed to create workingDirectory or outputDirectory";
			throw FileAccessError(message.str());
		}

		//Copy resources to working directory
		spdlog::trace("Copying Resources");
		for (string resourceFile : batchConfiguration["resources"]) {
			filesystem::path resourceFilePath(resourceFile);
			if (resourceFilePath.is_relative()) {
				resourceFilePath = batchConfiguration["workingDirectory"].get<string>();
				resourceFilePath.append(resourceFile);
			}
			spdlog::trace("Loading resource file {}", resourceFilePath.string());
			//Get target Path
			filesystem::path targetFilePath(workingDirectory);
			targetFilePath.append(resourceFilePath.filename().string());

			//Check if file is already there
			error_code ec;
			if (!filesystem::equivalent(targetFilePath, resourceFilePath, ec)) {
				ifstream input(resourceFilePath, ios::in | ios::binary);
				if (!input) {
					stringstream message;
					message << "Error reading resource file " << resourceFilePath.string();
					throw FileAccessError(message.str());
				}
				ofstream output(targetFilePath, ios::out | ios::binary);
				if (!input) {
					stringstream message;
					message << "Error writing resource file " << targetFilePath.string();
					throw FileAccessError(message.str());
				}
				output << input.rdbuf();
				output.close();
				input.close();
			}
		}
	} catch (const nlohmann::detail::exception&) {
		stringstream message;
		message << "Error accessing json";
		throw InvalidConfigurationError(message.str());
	}
}

Batch::~Batch()
{
}

vector<string> Batch::getOutputFiles() const
{
	return outputFiles;
}
