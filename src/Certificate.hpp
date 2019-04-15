#include <string>

using namespace std;

class Certificate{
private:
	string name;
	string content;
public:
	Certificate(const string& name, const string& content);
	const string getName() const;
	const string getContent() const;
}
