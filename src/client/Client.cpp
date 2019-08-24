#include "gen-cpp/CertificateGenerator.h"
#include "gen-cpp/CertificateGenerator_types.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#define BATCH_CONFIGURATION "resources/containerbatch.json"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace ::CertificateGeneratorThrift;
using namespace std;

int main(int argc, char **argv) {
	//Initialize thrift connection
	std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
	std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	CertificateGeneratorClient client(protocol);
	
	//Load configuration file
	stringstream file;
	ifstream input;
	input.open(BATCH_CONFIGURATION, ios::in);
	if(!input){
		cerr << "Error reading batch config" << endl;
		exit(EXIT_FAILURE);
	}
	file << input.rdbuf();
	
	//Open thrift connection
	transport->open();

	//Generate certificate
	std::string response;
	client.generateCertificate(response, file.str());
	std::cout << "Response: " << std::endl << response << std::endl;
	
	//Close thrift connection
	transport->close();
	
	//Write file to disk
	ofstream output("output.pdf", ios::out | ios::binary );
	output << response;
	output.close();
	
	return 0;
}
