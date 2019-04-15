#include "Batch.hpp"

	Batch::Batch(vector<Student> students, vector<TemplateCertificate> templateCertificates):
		students(students), templateCertificates(templateCertificates){
	}
	bool Batch::check() const{
		for(TemplateCertificate templateCertificate: templateCertificates){
			for(Student student: students){
				if(!templateCertificate.check(student)){
					return false;
				}
			}
		}
		return true;
	}
	void Batch::generateCertificates(){
		//Zeit messen und wenn nötig Multithreading
		for(TemplateCertificate templateCertificate: templateCertificates){
			for(Student student: students){
				certificates.push_back(templateCertificate.generateCertificate(student));
			}
		}
	}
	void Batch::outputCertificates() const{
		//Zeit messen und wenn nötig Multithreading
		for(Certificate certificate : certificates){
			//Todo pdflatex aufrufen
		}
	}
	void Batch::executeBatch(){
		generateCertificates();
		outputCertificates();
	}
