#ifndef BATCH_HPP
#define BATCH_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Student.hpp"
#include "TemplateCertificate.hpp"
#include "Certificate.hpp"
#include <filesystem>

using json = nlohmann::json;
using namespace std;

/**
 * @class Batch
 *
 * @brief A batch is a set of students and templates
 *
 * A batch is a set of students and templates.
 * You can create a batch either from a batch Configuration or
 * directly from a set of students and templates
 * 
 * If the batch is executed, certificates from every template
 * will be generated for every student
 */
class Batch{

private:
	vector<Student> students;
	vector<TemplateCertificate> templateCertificates;
	vector<Certificate> certificates;
	vector<string> outputFiles;
	string workingDirectory;
	string outputDirectory;
	void generateCertificates();
	void outputCertificates();

public:
	/** @brief Constructor that creates a Batch
    * @param [in] students is a vector of Student
    * @param [in] templateCertificates is a vector of TemplateCertificate
    * @param [in] workingDirectory a string specifying the directory to be used for temporary files
    * @param [in] outputDirectory a string specifying the directory where the pdf should be put
    * @return A pointer to the created Batch
    *
    * This method creates a Batch.
    */
	Batch(vector<Student> students, vector<TemplateCertificate> templateCertificates, const string& workingDirectory, const string& outputDirectory);
	
	/** @brief Constructor that creates a Batch from a json
    * @param [in] batchConfiguration is a json containing the configuration values
    * @return A pointer to the created Batch
    *
    * This method creates a Batch.
    * It loads the Students, the TemplateCertificates, the workingDirectory 
    * and the outputDirectory from the batchConfiguration
    */
	Batch(json batchConfiguration);
	
	/** @brief Destructor that also deletes the working directory
    *
    * This method destroys a Batch and deletes the working directory
    */
	~Batch();
	
	/** @brief This method checks whether the batch is valid
    * @return Boolean that indicates whether the batch is valid
    *
    * This method checks if every Student is compatiple with every TemplateCertificate
    */
	bool check() const;
	
	/** @brief This method executes this batch
    *
    * This method will generate the Certificates and
    * compile them to PDFs in the output folder.
    */
	void executeBatch();
	
	/** @brief This method returns the locations of the generated PDF files
    * @return vector<string> containing strings with the path of every output PDF
    * This method returns the locations of the generated PDF files.
    * 
    */
	vector<string> getOutputFiles() const;
};

#endif
