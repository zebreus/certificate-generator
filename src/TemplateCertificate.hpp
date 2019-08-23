#ifndef TEMPLATE_CERTIFICATE_HPP
#define TEMPLATE_CERTIFICATE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "Certificate.hpp"
#include "Student.hpp"
#include "thrift/CertificateGenerator_types.h"

using namespace  ::CertificateGeneratorThrift;

using json = nlohmann::json;
using namespace std;

/**
 * @class TemplateCertificate
 *
 * @brief A TemplateCertificate can generate certificates from a template file
 *
 * A TemplateCertificate can generate certificates from a template file and a student
 * You can also check if it is possible to generate this template is compatible with a given student
 * 
 * Template file syntax:
 * <local name="NAME OF FIELD" />    will be replaced with field from student
 *
 */

struct tagPosition{
	size_t start;
	size_t stop;
};

class TemplateCertificate{
	
private:
	json requiredProperties;
	json globalProperties;
	string templateContent;
	
	/** @brief This method returns a string that replaces the given optional field
    * @param [in] optional is a string representing the optional field
    * @param [in] student is the student containing the data with which the field will be filled
    * @return String that contains the replaced optional field
    *
    * This method returns you the string, which replaces the optional field 
    */
	string replaceOptional(const string& optional, const Student& student) const;
	
	/** @brief This method finds the next optional field in a given string
    * @param [in] full is the string that will be searched
    * @param [in] start is a int, that specifies at which position the search starts
    * @return tagPosition containing the beginning and end of the found optional. tagPosition.start is std::string::npos if no optional was found.
    *
    * This method finds the next optional field in full after start.
    */
	tagPosition findOptional(const string& full, int start) const;
	
	/** @brief This method finds the next substitution in a given string
    * @param [in] full is the string that will be searched
    * @param [in] start is a int, that specifies at which position the search starts
    * @return tagPosition containing the beginning and end of the found substitution. tagPosition.start is std::string::npos if no optional was found.
    *
    * This method finds the next substitution in full after start.
    */
	tagPosition findSubstitude(const string& full, int start) const;
	
	/** @brief This method extracts the content from a optional field
    * @param [in] optional a string containing the optional field
    * @return the content of the optional field
    *
    * This method extracts the content from a optional field
    */
	string getOptionalContent(const string& optional) const;
	
	/** @brief This method extracts the name from a optional field
    * @param [in] optional a string containing the optional field
    * @return the name of the optional field
    *
    * This method extracts the name from a optional field
    */
	string getOptionalName(const string& optional) const;
	
	/** @brief This method extracts the namespace from a optional field
    * @param [in] optional a string containing the optional field
    * @return the namespace of the optional field
    *
    * This method extracts the namespace from a optional field
    */
	string getOptionalNamespace(const string& optional) const;
	
	/** @brief This method extracts the name from a substitution
    * @param [in] substitude a string containing the substitution
    * @return the name of the substitution
    *
    * This method extracts the name from a substitution
    */
	string getSubstitudeName(const string& substitude) const;
	
	/** @brief This method extracts the namespace from a substitution
    * @param [in] substitude a string containing the substitution
    * @return the namespace of the substitution
    *
    * This method extracts the namespace from a substitution
    */
	string getSubstitudeNamespace(const string& substitude) const;
public:
	/** @brief Constructor that creates a TemplateCertificate
    * @param [in] template is a string containing the template for generated certificate
    * @return A pointer to the created TemplateCertificate
    *
    * description
    */
	TemplateCertificate(const string& templateContent, json& globalProperties);
	
	/** @brief This method checks whether the Student is compatible with this template
    * @param [in] student is the Student to be checked 
    * @return Boolean that indicates whether student is compatible
    *
    * This method checks whether student is compatible with this template
    */
	bool checkStudent(const Student& student) const;
	
	/** @brief This method generates a filename for a student
    * @param [in] student is the Student for whom a filename is generated
    * @return The generated filename
    *
    * This method generates a filename for a student
    */
	string generateName(const Student& student) const;
	
	/** @brief This method generates a certificate based on this template
    * @param [in] student is the Student to be used
    * @return The generated Certificate
    *
    * This method generates a Certificate based on this template and student
    */
	const Certificate generateCertificate(const Student& student);
};

#endif
