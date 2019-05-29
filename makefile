CPP  = g++
BIN  = certificate-generator
CFLAGS = $(INCS) -O0 -std=gnu++11
RM = rm -rf

build: certificate-generator

clean: 
	g++ --version
	${RM} bin/$(BIN) src/*.log src/*.o src/*.gcno src/*.gcda docs/* output/* working/*

certificate-generator: 
	$(CPP) -o bin/${BIN} $(CFLAGS) -Wall src/*.cpp   

certificate-generator-debug: 
	$(CPP) -g -o bin/${BIN} $(CFLAGS) -Wall src/*.cpp   

certificate-generator-picky: 
	$(CPP) -o bin/${BIN} $(CFLAGS) -Wall -Wextra -pedantic -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual src/*.cpp 
	
Doku: 
	- doxygen Doxyfile
