#include "Student.hpp"

Student::Student(json properties): properties(properties){
}
json Student::getProperties(){
	//TODO properties sichern
	return properties;
}
void Student::addCertificate(const TemplateCertificate& temp){
	Certificate c = temp.generateCertificate(this);
	certificates.push_back(c);
}
vector<Certificate> Student::getCertificates(){
	return certificates(;
}
