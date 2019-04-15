#include "TemplateCertificate.hpp"

TemplateCertificate::TemplateCertificate(const string& templateContent): templateContent(templateContent){
	//TODO parse required properties
}
bool TemplateCertificate::checkProperties(const json& properties) const{
	//TODO check properties
	return true;
}
const Certificate TemplateCertificate::generateCertificate(const json& properties) const{
	string cert = templateContent;
	//TODO generate real certificate
	//TODO think about naming
	return Certificate("NAME", cert);
}
