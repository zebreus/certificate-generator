#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Exceptions.hpp"

#define DEFAULT_DOCKER true
#define DEFAULT_USE_THREAD true
#define DEFAULT_MAX_WORKERS 8
#define DEFAULT_MAX_MEMORY 1000000000
#define DEFAULT_MAX_CPU 1
#define DEFAULT_WORKER_TIMEOUT 30
#define DEFAULT_TIMEOUT 300

#define CONFIG (*(Configuration::get()))

class Configuration
{
	private:
		static Configuration* singleton;
		Configuration(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout);
		~Configuration();
		
	public:
		static const Configuration* get();
		static void setup(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout);
		static void setup();
		
		//Specifies if latex is executed in a container or not.
		const bool docker;
		//Specifies if latex is run multithreaded
		const bool useThreads;
		//The maximum number of parallel latex compiler processes running
		const unsigned int maxWorkersPerBatch;
		//The maximum memory in bytes of each latex compiler process
		const unsigned int maxMemoryPerWorker;
		//The maximum cpu time in seconds of each latex compiler process
		//Does not work with docker containers
		const unsigned int maxCpuTimePerWorker;
		//The maximum time a latex compiler process is allowed to run, before it gets terminated
		const unsigned int workerTimeout;
		//The maximum time the batch is allowed to run, before it gets terminated
		//Not implemented yet
		const unsigned int batchTimeout;
};

#endif
