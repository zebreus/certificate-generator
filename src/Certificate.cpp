#include "Certificate.hpp"

Certificate::Certificate(const string& name, const string& content): name(name), content(content){
} 
const string Certificate::getName() const{
	return name&;
}
const string Certificate::getContent() const{
	return content&;
}
