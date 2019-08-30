#include "Configuration.hpp"

Configuration* Configuration::singleton = nullptr;

Configuration::Configuration(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout):
docker(docker), useThreads(useThreads), maxWorkersPerBatch(maxWorkersPerBatch), maxMemoryPerWorker(maxMemoryPerWorker), maxCpuTimePerWorker(maxCpuTimePerWorker), workerTimeout(workerTimeout), batchTimeout(batchTimeout){
}

Configuration::~Configuration(){
}

const Configuration* Configuration::get(){
	if(singleton == nullptr){
		setup();
	}
	return singleton;
}

void Configuration::setup(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout){
	if(singleton == nullptr){
		singleton = new Configuration(docker, useThreads, maxWorkersPerBatch, maxMemoryPerWorker, maxCpuTimePerWorker, workerTimeout, batchTimeout);
	}else{
		throw ConfigurationError("Configuration already specified");
	}
}

void Configuration::setup(){
	if(singleton == nullptr){
		singleton = new Configuration(DEFAULT_DOCKER, DEFAULT_USE_THREAD, DEFAULT_MAX_WORKERS, DEFAULT_MAX_MEMORY, DEFAULT_MAX_CPU, DEFAULT_WORKER_TIMEOUT, DEFAULT_TIMEOUT);
	}else{
		throw ConfigurationError("Configuration already specified");
	}
}

