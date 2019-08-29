#include "TemplateCertificate.hpp"


TemplateCertificate::TemplateCertificate(const string& basename, const string& templateContent, json& globalProperties): globalProperties(globalProperties), templateContent(templateContent), basename(basename), generatedCertificateCounter(0) {
	
}



bool TemplateCertificate::checkStudent(const Student& student) const{
	return true;
}


const Certificate TemplateCertificate::generateCertificate(const Student& student){
	generatedCertificateCounter++;
	string result = templateContent;
	
	//replace use dummy package
	long unsigned int useDummyPackage = result.find("\\usepackage{certificate-generator}");
	if(useDummyPackage != string::npos){
		result.replace(useDummyPackage, 34, "");
	}
	
	//replace optional
	for(tagPosition tp = findOptional(result, 0); tp.start != string::npos; tp = findOptional(result, tp.start+1)){
		string optional = result.substr(tp.start,tp.stop-tp.start);
		result.replace(tp.start,tp.stop-tp.start+1,replaceOptional(optional, student));
	}
	
	
	//replace tags
	for(tagPosition tp = findSubstitude(result, 0); tp.start != string::npos; tp = findSubstitude(result, tp.start+1)){
		string substitution = result.substr(tp.start,tp.stop-tp.start);
		string substitutionNamespace = getSubstitudeNamespace(substitution);
		string substitutionName = getSubstitudeName(substitution);
		if(substitutionNamespace == "student"){
			if(student.getProperties()[substitutionName].is_string()){
				result.replace(tp.start,tp.stop-tp.start+1,student.getProperties()[substitutionName]);
			}else{
				stringstream errormessage;
				errormessage << "No property " << substitutionName << " of type string in " << substitutionNamespace;
				throw InvalidConfigurationError(errormessage.str());
			}
		}else if(substitutionNamespace == "global"){
			if(globalProperties[substitutionName].is_string()){
				result.replace(tp.start,tp.stop-tp.start+1,globalProperties[substitutionName]);
			}else{
				stringstream errormessage;
				errormessage << "No property " << substitutionName << " of type string in " << substitutionNamespace;
				throw InvalidConfigurationError(errormessage.str());
			}
		}else if(substitutionNamespace == "auto"){
			if(student.getProperties()[substitutionName].is_string()){
				result.replace(tp.start,tp.stop-tp.start+1,student.getProperties()[substitutionName]);
			}else if(globalProperties[substitutionName].is_string()){
				result.replace(tp.start,tp.stop-tp.start+1,globalProperties[substitutionName]);
			}else{
				stringstream errormessage;
				errormessage << "No property " << substitutionName << " of type string in any valid namespace";
				throw InvalidConfigurationError(errormessage.str());
			}
		}
	}
	
	string filename = generateName(student);
	return Certificate(filename, result);
}


string TemplateCertificate::generateName(const Student& student) const{
	stringstream name;
	name << basename << "_" << generatedCertificateCounter;
	if(student.getProperties()["surname"] != nullptr){
		name << "_" << student.getProperties()["surname"].get<string>();
	}
	if(student.getProperties()["name"] != nullptr){
		name << "_" << student.getProperties()["name"].get<string>();
	}
	return name.str();
}

//TODO tables not only in student
string TemplateCertificate::replaceOptional(const string& optional, const Student& student) const{
	string result;
	json name = student.getProperties()[getOptionalName(optional)];
	string content = getOptionalContent(optional);
	if(name.is_array()){
		for(json object : name){
			string line = content;
			for(tagPosition tp = findSubstitude(line, 0); tp.start != string::npos; tp = findSubstitude(line, tp.start+1)){
				string substitution = line.substr(tp.start,tp.stop-tp.start);
				string substitutionNamespace = getSubstitudeNamespace(substitution);
				string substitutionName = getSubstitudeName(substitution);
				//TODO detect also "table" as a valid namespace and throw a error if appropriate
				if(substitutionNamespace == "auto"){
					if(object[substitutionName].is_string()){
						line.replace(tp.start, tp.stop-tp.start+1, object[substitutionName]);
					}
				}
			}
			result.append(line);
		}
	}else{
		stringstream errormessage;
		errormessage << "No array " << getOptionalName(optional) << " in student";
		throw InvalidConfigurationError(errormessage.str());
	}
	return result;
}

tagPosition TemplateCertificate::findOptional(const string& full, int start) const{
	tagPosition tp;
	tp.start = full.find("\\optional", start);
	tp.stop = full.find("}",tp.start);
	
	int open = full.find("{", tp.stop);
	int close = open+1;
	while(open < close){
		open = full.find("{", open+1);
		close = full.find("}", close+1);
	}
	
	tp.stop = close+1;
	
	return tp;
}

tagPosition TemplateCertificate::findSubstitude(const string& full, int start) const{
	tagPosition tp;
	tp.start = full.find("\\substitude", start);
	tp.stop = full.find("}",tp.start);
	return tp;
}

string TemplateCertificate::getOptionalContent(const string& optional) const{
	int start = optional.find("{", optional.find("{")+1);
	return optional.substr(start+1,optional.length()-(start)-2);
}

string TemplateCertificate::getOptionalName(const string& optional) const{
	tagPosition tp;
	tp.start = optional.find("{") + 1;
	tp.stop = optional.find("}", tp.start );
	return optional.substr(tp.start, tp.stop-tp.start );
}

string TemplateCertificate::getOptionalNamespace(const string& optional) const{
	tagPosition tp;
	tp.start = optional.find("[");
	if(tp.start != string::npos){
		tp.stop = optional.find("]", tp.start );
		return optional.substr(tp.start+1, tp.stop-(tp.start+1) );
	}else{
		return "auto";
	}
}

string TemplateCertificate::getSubstitudeName(const string& substitude) const{
	tagPosition tp;
	tp.start = substitude.find("{") + 1;
	tp.stop = substitude.find("}", tp.start );
	return substitude.substr(tp.start, tp.stop-tp.start );
}

string TemplateCertificate::getSubstitudeNamespace(const string& substitude) const{
	tagPosition tp;
	tp.start = substitude.find("[");
	if(tp.start != string::npos){
		tp.stop = substitude.find("]", tp.start );
		return substitude.substr(tp.start+1, tp.stop-(tp.start+1) );
	}else{
		return "auto";
	}
}
