#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "Student.hpp"
#include "TemplateCertificate.hpp"
#include "Certificate.hpp"

using json = nlohmann::json;
using namespace std;

class Batch{
private:
	vector<Student> students;
	vector<TemplateCertificate> templateCertificates;
	vector<Certificate> certificates;
	void generateCertificates();
	void outputCertificates() const;
public:
	Batch(vector<Student> students, vector<TemplateCertificate> templateCertificates);
	bool check() const;
	void executeBatch();
}
