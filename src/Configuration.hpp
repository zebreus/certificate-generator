#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Exceptions.hpp"

#define DEFAULT_DOCKER true
#define DEFAULT_USE_THREAD true
#define DEFAULT_MAX_BATCH_WORKERS 8
#define DEFAULT_MAX_MEMORY 1000000000
#define DEFAULT_MAX_CPU 10
#define DEFAULT_WORKER_TIMEOUT 30
#define DEFAULT_TIMEOUT 300
#define DEFAULT_MAX_WORKERS 8

#define MTOS_HELPER(m) #m
#define MTOS(m) MTOS_HELPER(m)

//A handy macro to get the Configuration object
#define CONFIG (*(Configuration::get()))

/**
 * @class Configuration
 *
 * @brief The global configuration of the certificate generator
 *
 * The global configuration of the certificate generator.
 * It can be set once and is immutable afterwards.
 */
class Configuration {
private:
	static Configuration* singleton;
	
	/** @brief Constructor that creates a Configuration
    * @param [in] docker a string specifying if latex is executed in a container or not
    * @param [in] useThreads a bool specifying if latex is run multithreaded
    * @param [in] maxWorkersPerBatch a int specifying the maximum number of parallel latex compiler processes running per batch
    * @param [in] maxMemoryPerWorker a int specifying the maximum memory in bytes of each latex compiler process
    * @param [in] maxCpuTimePerWorker a int specifying the maximum cpu time in seconds of each latex compiler process
    * @param [in] workerTimeout a int specifying the maximum time a latex compiler process is allowed to run, before it gets terminated
    * @param [in] batchTimeout a int specifying the maximum time the batch is allowed to run, before it gets terminated
    * @param [in] maxWorkers a int specifying the maximum number of parallel latex compiler processes running
    * @return A pointer to the created Certificate
    *
    * This method creates a configuration with the given parameters
    * 
    * Its private, to prevent other classes to create a Configuration
    * object other than the one singleton points to.
    */
	Configuration(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout, unsigned int maxWorkers);
	
	/** @brief Destructor of Configuration
    *
    * This method destroys the Configuration.
    * 
    * Its private, to prevent destruction of the Configuration.
    */
	~Configuration();

public:
	/** @brief Returns a pointer to the configuration singleton object
    * @return A pointer to the configuration singleton object
    * Returns a pointer to the configuration singleton object.
    * 
    * If no configuration object exists, it will create a new one with the default values
    */
	static const Configuration* get();
	
	/** @brief Generates a Configuration and sets the singleton
    * @param [in] docker a string specifying if latex is executed in a container or not
    * @param [in] useThreads a bool specifying if latex is run multithreaded
    * @param [in] maxWorkersPerBatch a int specifying the maximum number of parallel latex compiler processes running per batch
    * @param [in] maxMemoryPerWorker a int specifying the maximum memory in bytes of each latex compiler process
    * @param [in] maxCpuTimePerWorker a int specifying the maximum cpu time in seconds of each latex compiler process
    * @param [in] workerTimeout a int specifying the maximum time a latex compiler process is allowed to run, before it gets terminated
    * @param [in] batchTimeout a int specifying the maximum time the batch is allowed to run, before it gets terminated
    * @param [in] maxWorkers a int specifying the maximum number of parallel latex compiler processes running
    * @throw ConfigurationError if the singleton is already set
    * Generates a Configuration with the given values and sets the singleton to it.
    * 
    * Throws a ConfigurationError if the singleton is already set.
    */
	static void setup(bool docker, bool useThreads, unsigned int maxWorkersPerBatch, unsigned int maxMemoryPerWorker, unsigned int maxCpuTimePerWorker, unsigned int workerTimeout, unsigned int batchTimeout, unsigned int maxWorkers);
	/** @brief Generates a Configuration and sets the singleton
	* @throw ConfigurationError if the singleton is already set
    * Generates a Configuration with the default values and sets the singleton to it.
    * 
    * Throws a ConfigurationError if the singleton is already set.
    */
	static void setup();

	//Specifies if latex is executed in a container or not.
	const bool docker;
	//Specifies if latex is run multithreaded
	const bool useThreads;
	//The maximum number of parallel latex compiler processes running per batch
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
	//The maximum number of parallel latex compiler processes running
	const unsigned int maxWorkers;
};

#endif
