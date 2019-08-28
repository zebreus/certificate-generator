#ifndef CERTIFICATE_HPP
#define CERTIFICATE_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <wait.h>
#include <cstring>
#include <iostream>
#include "Exceptions.hpp"

using namespace std;

/**
 * @class Certificate
 *
 * @brief A certificate represents a generated certificate
 *
 * A certificate stores a generated certificate
 * You can generate a PDF file from it
 */
class Certificate{

private:
	string name;
	string content;

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
    * @return A string containing the location of the PDF file.
    * Generates a pdf of the certificate into the given outputDirectory
    * 
    * The workingDirectory is used to store temporary files needed 
    * during the generation of the certificate. The temporary files
    * can be removed after this method finished.
    */
	string generatePDF(const string& workingDirectory,const string& outputDirectory) const;
};

#endif
