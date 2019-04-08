#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using json = nlohmann::json;
using namespace std;

#define TEMPLATE_FILE "template.tex"
#define BATCH_CONFIG "batch.json"


struct certificate_t{
	string filename;
	string content;
};

bool replace(string&, const string&, const string&);

int main(int argc, char** argv){
	//Load json batch
	ifstream input;
	input.open(BATCH_CONFIG, ios::in);
	if(!input){
		cerr << "Error" << endl;
	}
	json batch = json::parse(input);
	input.close();
	
	//TODO make template certificate_t
	//Load template
	input.open(TEMPLATE_FILE, ios::in);
	std::string templateCertificateContent( (std::istreambuf_iterator<char>(input) ),
                       (std::istreambuf_iterator<char>()) );
	input.close();
	
	//Make simple changes
	replace(templateCertificateContent,"|||signee|||",batch["signee"]);
	
	//Build output texts
	vector<certificate_t> certificates;
	for(json person : batch["people"]){
		//Create certificate
		certificate_t certificate;
		certificate.content = templateCertificateContent;
		
		//Set filename
		certificate.filename = person["surname"];
		certificate.filename.append("_");
		certificate.filename.append(person["name"]);
		certificate.filename.append(".tex");
		
		//Replace single words
		replace(certificate.content,"|||name|||",person["name"]);
		replace(certificate.content,"|||surname|||",person["surname"]);
		replace(certificate.content,"|||date|||",person["date"]);
		
		//Build achievement string
		string achievements;
		for(json achievement : person["achievements"]){
			achievements.append(achievement["name"]);
			achievements.append(" & ");
			achievements.append(achievement["grade"]);
			achievements.append(" \\\\ \\hline ");
		}
		cout << replace(certificate.content,"|||achievements|||",achievements);
		cout << certificate.content << endl;
		
		certificates.push_back(certificate);
	}
	
	//Write .tex files
	ofstream output;
	for( certificate_t cert : certificates){
		output.open(cert.filename, ios::out);
		output << cert.content;
		output.close();
	}
	
	//cout << templatefile << " | " << templatefile.length() << endl;
	
	//cout << batch["people"][0]["name"] << endl;
}

//Function to replace string in string
//TODO upgrade to replaceall
bool replace(string& work, const string& search, const string& replace){
	size_t start = work.find(search);
	if(start != string::npos){
		work.replace(start,search.length(),replace);
		return true;
	}
	return false;
}
