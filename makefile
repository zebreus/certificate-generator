CPP  = g++
THRIFT = thrift --gen cpp
THRIFTFILE = CertificateGenerator
BIN  = certificate-generator
CFLAGS = $(INCS) -O0 -std=gnu++11
CTHRIFT = -I/usr/local/include/thrift
RM = rm -rf

build: certificate-generator

src/Certificate.o: src/Certificate.cpp src/Certificate.hpp
	$(CPP) $(CFLAGS) -c src/Certificate.cpp -o src/Certificate.o

src/Batch.o: src/Batch.cpp src/Batch.hpp
	$(CPP) $(CFLAGS) -c src/Batch.cpp -o src/Batch.o

src/TemplateCertificate.o: src/TemplateCertificate.cpp src/TemplateCertificate.hpp
	$(CPP) $(CFLAGS) -c src/TemplateCertificate.cpp -o src/TemplateCertificate.o

src/Student.o: src/Student.cpp src/Student.hpp
	$(CPP) $(CFLAGS) -c src/Student.cpp -o src/Student.o

src/Launcher.o: src/Launcher.cpp
	$(CPP) $(CFLAGS) -c src/Launcher.cpp -o src/Launcher.o
	
src/CertificateGenerator_server.o: src/CertificateGenerator_server.cpp
	$(CPP) $(CFLAGS) $(CTHRIFT) -c src/CertificateGenerator_server.cpp -o src/CertificateGenerator_server.o

src/CertificateGenerator_client.o: src/CertificateGenerator_client.cpp
	$(CPP) $(CFLAGS) $(CTHRIFT) -c src/CertificateGenerator_client.cpp -o src/CertificateGenerator_client.o

src/thrift/CertificateGenerator.o: src/thrift/CertificateGenerator.cpp src/thrift/CertificateGenerator.h
	$(CPP) $(CFLAGS) $(CTHRIFT) -c src/thrift/CertificateGenerator.cpp -o src/thrift/CertificateGenerator.o

src/thrift/CertificateGenerator_constants.o: src/thrift/CertificateGenerator_constants.cpp src/thrift/CertificateGenerator_constants.h
	$(CPP) $(CFLAGS) $(CTHRIFT) -c src/thrift/CertificateGenerator_constants.cpp -o src/thrift/CertificateGenerator_constants.o

src/thrift/CertificateGenerator_types.o: src/thrift/CertificateGenerator_types.cpp src/thrift/CertificateGenerator_types.h
	$(CPP) $(CFLAGS) $(CTHRIFT) -c src/thrift/CertificateGenerator_types.cpp -o src/thrift/CertificateGenerator_types.o

certificate-generator: src/Certificate.o src/Batch.o src/Student.o src/TemplateCertificate.o src/Launcher.o
	$(CPP) -o bin/${BIN} $(CFLAGS) src/Certificate.o src/Student.o src/TemplateCertificate.o src/Batch.o src/Launcher.o

certificate-generator-server: src/Certificate.o src/Batch.o src/Student.o src/TemplateCertificate.o src/CertificateGenerator_server.o src/thrift/CertificateGenerator.o src/thrift/CertificateGenerator_constants.o src/thrift/CertificateGenerator_types.o
	$(CPP) -o bin/${BIN}-server $(CFLAGS) -Wall -I/usr/local/include/thrift src/Certificate.o src/Student.o src/TemplateCertificate.o src/Batch.o src/CertificateGenerator_server.o src/thrift/CertificateGenerator_constants.o src/thrift/CertificateGenerator_types.o src/thrift/CertificateGenerator.o -L/usr/local/lib -lthrift

certificate-generator-client: src/Certificate.o src/Batch.o src/Student.o src/TemplateCertificate.o src/CertificateGenerator_client.o src/thrift/CertificateGenerator.o src/thrift/CertificateGenerator_constants.o src/thrift/CertificateGenerator_types.o
	$(CPP) -o bin/${BIN}-client $(CFLAGS) -Wall -I/usr/local/include/thrift src/Certificate.o src/Student.o src/TemplateCertificate.o src/Batch.o src/CertificateGenerator_client.o src/thrift/CertificateGenerator_constants.o src/thrift/CertificateGenerator_types.o src/thrift/CertificateGenerator.o -L/usr/local/lib -lthrift


clean: 
	g++ --version
	${RM} bin/$(BIN) src/*.log src/*.o src/*.gcno src/*.gcda docs/* output/* working/*

#certificate-generator: 
#	$(CPP) -o bin/${BIN} $(CFLAGS) -Wall src/*.cpp   

#certificate-generator-thrift: 
#	$(CPP) -o bin/${BIN}-thrift -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -Wall -I/usr/local/include/thrift src/*.cpp -L/usr/local/lib -lthrift -o something 

#certificate-generator-debug: 
#	$(CPP) -g -o bin/${BIN} $(CFLAGS) -Wall src/*.cpp   

#certificate-generator-picky: 
#	$(CPP) -o bin/${BIN} $(CFLAGS) -Wall -Wextra -pedantic -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual src/*.cpp 
	
doku: 
	- doxygen Doxyfile

thrift:
	rm -rf src/thrift
	mkdir src/thrift/
	$(THRIFT) -out src/thrift/ src/$(THRIFTFILE).thrift
