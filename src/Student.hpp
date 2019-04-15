#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;

class Student{
private:
	json properties;
	vector<Certificate> certificates;
public:
	Student(json properties);
	json getProperties();
	bool addCertificate(const TemplateCertificate&);
	vector<Certificate> getCertificates();
}
