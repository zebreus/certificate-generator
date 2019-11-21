#include "gtest/gtest.h"

#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>

#define protected public
#define private public

#include "Certificate.hpp"

#undef protected
#undef private


using namespace std;

class CertificateTest : public ::testing::Test {
protected:
	shared_ptr<Certificate> testCertificate;
	string testContent =
		"\\documentclass[a4paper]{article} "
		"\\title{Example} "
		"\\begin{document} "
		"\\maketitle "
		"\\section{Section} "
		"This is \\LaTeX. "
		"\\end{document} ";
	string testName = "testName";
	
	CertificateTest()
	{
	}

	~CertificateTest() override
	{
	}

	void SetUp() override
	{
		testCertificate = make_shared<Certificate>("testName", testContent);
	}
	
	void TearDown() override
	{
		if(gotWorkingDirectory){
			try{
				filesystem::remove_all(workingDirectory);
			}catch(...){
			}
		}
	}
	
	//Get a directory that will be removed after test.
	filesystem::path getWorkingDirectory(){
		if(!gotWorkingDirectory){
			workingDirectory = filesystem::temp_directory_path();
			workingDirectory.append("certificateTest");
			filesystem::create_directories(workingDirectory);
			gotWorkingDirectory = true;
		}
		return workingDirectory;
	}
	
private:
	filesystem::path workingDirectory;
	bool gotWorkingDirectory = false;
};

// Tests that the Certificate::getName() method returns the expected name.
TEST_F(CertificateTest, GetNameWorks)
{
	EXPECT_EQ(testCertificate->getName(), testName);
}

// Tests that the Certificate::getContent() method returns the expected content.
TEST_F(CertificateTest, GetContentWorks)
{
	EXPECT_EQ(testCertificate->getContent(), testContent);
}

// Tests that the Certificate::Certificate sets name and content
TEST_F(CertificateTest, ConstructorWorks)
{
	Certificate certificate("NAME", "CONTENT");
	
	EXPECT_EQ(certificate.getName(), "NAME");
	EXPECT_EQ(certificate.getContent(), "CONTENT");
}

// Tests that the Certificate::writeToWorkingDirectory produces a file with the correct name
TEST_F(CertificateTest, WriteToWorkingDirectoryCorrectFilename)
{
	filesystem::path directory = getWorkingDirectory();
	
	Certificate noExtension = *testCertificate;
	Certificate wrongExtension = *testCertificate;
	Certificate correctExtension = *testCertificate;
	noExtension.name = "no_extension";
	wrongExtension.name = "wrong_extension.wrong";
	correctExtension.name = "correct_extension.tex";
	
	noExtension.writeToWorkingDirectory(directory);
	wrongExtension.writeToWorkingDirectory(directory);
	correctExtension.writeToWorkingDirectory(directory);
	filesystem::path noExtensionFile = directory;
	filesystem::path wrongExtensionFile = directory;
	filesystem::path correctExtensionFile = directory;
	
	noExtensionFile.append("no_extension.tex");
	wrongExtensionFile.append("wrong_extension.tex");
	correctExtensionFile.append("correct_extension.tex");
	
	EXPECT_TRUE(filesystem::exists(noExtensionFile)) << "File without an extension is saved under wrong name.";
	EXPECT_TRUE(filesystem::exists(wrongExtensionFile)) << "File with a wrong extension is saved under wrong name.";
	EXPECT_TRUE(filesystem::exists(correctExtensionFile)) << "File with a correct(.tex) extension is saved under wrong name.";
}

// Tests that the Certificate::writeToWorkingDirectory writes a file with the correct content
TEST_F(CertificateTest, WriteToWorkingDirectoryCorrectContent)
{
	filesystem::path directory = getWorkingDirectory();
	testCertificate->writeToWorkingDirectory(directory);
	filesystem::path file = directory;
	file.append(testCertificate->getName());
	file.replace_extension(".tex");
	
	ASSERT_TRUE(filesystem::exists(file)) << "File does not exist";
	
	ifstream latexFile;
	latexFile.open(file);
	stringstream fileContentStream;
	fileContentStream << latexFile.rdbuf();
	string fileContent = fileContentStream.str();
	
	ASSERT_EQ(testCertificate->getContent(), fileContent);
}

// Tests that the Certificate::moveResultToOutputDirectory moves the correct file to the output directory
TEST_F(CertificateTest, moveResultToOutputDirectoryWorks)
{
	//TODO create better test
	//Create working and output directory path
	filesystem::path directory = getWorkingDirectory();
	filesystem::path outputDirectory = directory;
	outputDirectory.append("output");
	filesystem::create_directories(outputDirectory);
	
	//Create path to file in working directory with the same name as the generated pdf
	filesystem::path originalFile = directory;
	originalFile.append(testCertificate->getName());
	originalFile.replace_extension(".pdf");
	
	//Create path to file in output directory with the same name as the generated pdf
	filesystem::path outputFile = outputDirectory;
	outputFile.append(testCertificate->getName());
	outputFile.replace_extension(".pdf");
	
	//Write content to original file
	ofstream originalFileStream(originalFile);
	originalFileStream << "CONTENT" << flush;
	originalFileStream.close();
	
	testCertificate->moveResultToOutputDirectory(directory, outputDirectory);
	
	//Check if output file got created
	ASSERT_TRUE(filesystem::exists(outputFile)) << "File does not exist";
	
	//Read moved/copied file
	ifstream outputFileStream(outputFile);
	stringstream outputFileContent;
	outputFileContent << outputFileStream.rdbuf();
	
	ASSERT_EQ(outputFileContent.str(), "CONTENT");
}
