#include <nlohmann/json.hpp>
#include <string>
#include "Certificate.hpp"

using json = nlohmann::json;
using namespace std;

class TemplateCertificate{
private:
	json requiredProperties;
	string templateContent;
public:
	TemplateCertificate(const string& templateContent);
	bool checkProperties(const json& properties) const;
	const Certificate generateCertificate(const json& properties) const;
}
