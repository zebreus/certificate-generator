#include "gtest/gtest.h"

#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Exceptions.hpp"

#define protected public
#define private public

#include "Configuration.hpp"

#undef protected
#undef private


using namespace std;

class ConfigurationTest : public ::testing::Test {
protected:
	
	ConfigurationTest()
	{
	}

	~ConfigurationTest() override
	{
	}

	void SetUp() override
	{
		//Resets singleton to avoid influence from previous test
		Configuration::singleton = nullptr;
	}
	
	void TearDown() override
	{
		//Resets singleton to avoid influencing next test
		Configuration::singleton = nullptr;
	}

};

// Tests that CONFIG Makro returns a configuration;
TEST_F(ConfigurationTest, CONFIGWorks)
{
	EXPECT_EQ(Configuration::get(), &CONFIG);
}

// Tests that Configuration::get creates a new singleton if non is existant
TEST_F(ConfigurationTest, GetCreatesSingleton)
{
	EXPECT_NE(Configuration::get(), nullptr);
}

// Tests that Configuration::get returns no nullptr
TEST_F(ConfigurationTest, GetReturnsNoNullptr)
{
	EXPECT_NE(Configuration::get(), nullptr);
	EXPECT_NE(Configuration::get(), nullptr);
}

// Tests that Configuration::get does not recreate singleton if it already exists
TEST_F(ConfigurationTest, GetDoesNotChangeSingleton)
{
	Configuration::setup();
	EXPECT_EQ(Configuration::get(), Configuration::get());
}

// Tests that Configuration::setup throws ConfigurationError on second call
TEST_F(ConfigurationTest, setupThrowOnSecondCall)
{
	ASSERT_NO_THROW(Configuration::setup()) << "Threw an exception on first call";
	ASSERT_THROW(Configuration::setup(), ConfigurationError) << "Threw no ConfigurationError on second call";
}

// Tests that Configuration::setup sets the default values
TEST_F(ConfigurationTest, setupSetsDefaultValues)
{
	Configuration::setup();
	EXPECT_EQ(CONFIG.docker, DEFAULT_DOCKER);
	EXPECT_EQ(CONFIG.useThreads, DEFAULT_USE_THREAD);
	EXPECT_EQ(CONFIG.maxWorkersPerBatch, DEFAULT_MAX_BATCH_WORKERS);
	EXPECT_EQ(CONFIG.maxMemoryPerWorker, DEFAULT_MAX_MEMORY);
	EXPECT_EQ(CONFIG.maxCpuTimePerWorker, DEFAULT_MAX_CPU);
	EXPECT_EQ(CONFIG.workerTimeout, DEFAULT_WORKER_TIMEOUT);
	EXPECT_EQ(CONFIG.batchTimeout, DEFAULT_TIMEOUT);
	EXPECT_EQ(CONFIG.maxWorkers, DEFAULT_MAX_WORKERS);
}

// Tests that Configuration::setup sets the given values
TEST_F(ConfigurationTest, setupSetsGivenValues)
{
	Configuration::setup(!DEFAULT_DOCKER, !DEFAULT_USE_THREAD, 3453, 945, 4533, 748, 1348, 898);
	EXPECT_EQ(CONFIG.docker, !DEFAULT_DOCKER);
	EXPECT_EQ(CONFIG.useThreads, !DEFAULT_USE_THREAD);
	EXPECT_EQ(CONFIG.maxWorkersPerBatch, 3453);
	EXPECT_EQ(CONFIG.maxMemoryPerWorker, 945);
	EXPECT_EQ(CONFIG.maxCpuTimePerWorker, 4533);
	EXPECT_EQ(CONFIG.workerTimeout, 748);
	EXPECT_EQ(CONFIG.batchTimeout, 1348);
	EXPECT_EQ(CONFIG.maxWorkers, 898);
}

// Tests that Configuration::setup does not set values on second call
TEST_F(ConfigurationTest, setupDoesNotModifyValuesOnSecondCall)
{
	Configuration::setup(false, false, 0, 0, 0, 0, 0, 0);
	EXPECT_THROW( Configuration::setup(true, true, 1, 1, 1, 1, 1, 1), ConfigurationError );
	EXPECT_EQ(CONFIG.docker, false);
	EXPECT_EQ(CONFIG.useThreads, false);
	EXPECT_EQ(CONFIG.maxWorkersPerBatch, 0);
	EXPECT_EQ(CONFIG.maxMemoryPerWorker, 0);
	EXPECT_EQ(CONFIG.maxCpuTimePerWorker, 0);
	EXPECT_EQ(CONFIG.workerTimeout, 0);
	EXPECT_EQ(CONFIG.batchTimeout, 0);
	EXPECT_EQ(CONFIG.maxWorkers, 0);
}

// Tests that Configuration::get returns the right values
TEST_F(ConfigurationTest, getGetsRightValues)
{
	Configuration::setup(false, false, 0, 0, 0, 0, 0, 0);
	EXPECT_EQ(Configuration::get()->docker, false);
	EXPECT_EQ(Configuration::get()->useThreads, false);
	EXPECT_EQ(Configuration::get()->maxWorkersPerBatch, 0);
	EXPECT_EQ(Configuration::get()->maxMemoryPerWorker, 0);
	EXPECT_EQ(Configuration::get()->maxCpuTimePerWorker, 0);
	EXPECT_EQ(Configuration::get()->workerTimeout, 0);
	EXPECT_EQ(Configuration::get()->batchTimeout, 0);
	EXPECT_EQ(Configuration::get()->maxWorkers, 0);
}
