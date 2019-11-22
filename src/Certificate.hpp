#ifndef CERTIFICATE_HPP
#define CERTIFICATE_HPP

#include "Configuration.hpp"
#include "Exceptions.hpp"
#include <atomic>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wait.h>

using namespace std;

/**
 * @class Certificate
 *
 * @brief A certificate represents a generated certificate
 *
 * A certificate stores a generated certificate
 * You can generate a PDF file from it
 */
class Certificate {

private:
	string name;
	string content;
	
	/** @brief Writes the latex file to the given directory
    * @param [in] workingDirectory a string specifying the directory where the file should be placed
    * @return A string containing the location of the latex file.
    * 
    * Writes the latex file to workingDirectory. The filename will be
    * the certificate name with the extension .tex. If the certificate
    * name already has an extension it will be replaced.
    */
	void writeToWorkingDirectory(const filesystem::path& workingDirectory) const;
	
	/** @brief Moves generated pdf files from
    * @param [in] workingDirectory a string specifying the directory where the pdf to be copied is.
    * @param [in] outputDirectory a string specifying the directory where the pdf should be put
    * @return A string containing the location of the PDF file.
    * 
    * Moves the generated pdf file from workingDirectory to outputDirectory
    */
	filesystem::path moveResultToOutputDirectory(const filesystem::path& workingDirectory, const filesystem::path& outputDirectory) const;
	
	/** @brief Removes temporary files from the working directory
    * @param [in] workingDirectory a string specifying the directory where the temporary files are.
    * 
    * Removes the pdf, aux and tex files from the working directory
    */
	void cleanWorkingDirectory(const filesystem::path& workingDirectory) const;
	
	/** @brief Generates the arguments for execvp to execute latex
    * @return A vector of strings containing arguments.
    * 
    * Generates the arguments for execvp to execute latex
    */
	vector<string> generateLatexArguments(const filesystem::path& workingDirectory) const;
	
	/** @brief Executes a program
	* @param [in] arguments a vector of strings containing arguments.
    * 
    * Executes the program specified in arguments with execvp.
    * Before executing the program, the working directory is set to
    * workingDirectory the resources of the process are limited with
    * setrlimit, stdout is redirected to /dev/null
    * and niceness is increased
    */
	void executeProgram(vector<string> arguments, const filesystem::path& workingDirectory) const;
	
	/** @brief Waits for the process to finish or kills it
	* @param [in] childPid a pid_t of the process to be waited for
	* @param [in] killswitch a atomic_bool triggering the sending of a kill signal to the child
    * @return A int containing the exit status of the process
    * 
    * Waits until the process with childPid exits.
    * If the timeout set in Configuration is exceeded, the process with
    * childPid gets send a SIGTERM signal every 10ms. If it does not
    * terminate withing 2 seconds it gets send SIGKILL instead.
    * If killswitch gets set the process with childPid gets send SIGKILL
    * every 10ms.
    */
	int waitForProcess(const pid_t& childPid, const atomic_bool& killswitch) const;

public:
	/** @brief Constructor that creates a Certificate
    * @param [in] name is a string containing the name of the certificate without ending
    * @param [in] name is a string containing the content of the certificate
    * @return A pointer to the created Certificate
    *
    * This method creates a Certificate with the given filename and content
    */
	Certificate(const string& name, const string& content);

	/** @brief Returns the name of the Certificate
    * @return A string that containing the name of the certificate
    */
	const string getName() const;

	/** @brief Returns the content of the Certificate
    * @return A string that containing the content of the certificate
    */
	const string getContent() const;

	/** @brief Generates a pdf from the certificate
    * @param [in] workingDirectory a string specifying the directory to be used for temporary files
    * @param [in] outputDirectory a string specifying the directory where the pdf should be put
    * @param [in] killswitch a atomic_bool triggering cancelation of the generation, when set.
    * @return A string containing the location of the PDF file.
    * Generates a pdf of the certificate into the given outputDirectory
    * 
    * The workingDirectory is used to store temporary files needed 
    * during the generation of the certificate. The temporary files
    * can be removed after this method finished.
    * 
    * This will produce a pdf file in outputDirectory. The filename will be
    * the certificate name with the extension .pdf. If the certificate
    * name already has an extension it will be replaced with .pdf.
    * 
    * If killswitch is set by another thread, it returns as soon as possible
    * with an empty string.
    */
	filesystem::path generatePDF(const filesystem::path& workingDirectory, const filesystem::path& outputDirectory, const atomic_bool& killswitch) const;
};

#endif
