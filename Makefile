#Paths
OUTPUT = ./out/
MAIN = ./src/
SERVER = ./src/server/
CLIENT = ./src/client/
LOCAL = ./src/local/
NLOHMANN_JSON = ./libs/json/
CXXOPTS = ./libs/cxxopts/
SPDLOG = ./libs/spdlog/
THRIFT_GENERATED = $(MAIN)/gen-cpp/
RESOURCES = ./res/
TEST = ./test/
GENERATOR_TEST = ./test/unittest/generator/

#Configuration
CPP  = g++
THRIFT = thrift
THRIFTFILE = CertificateGenerator.thrift
CPPFLAGS = -Wall -Wformat -Os -std=c++17

#Sourcecode and flags
MAIN_SOURCES = $(MAIN)/Certificate.cpp $(MAIN)/Batch.cpp 
MAIN_SOURCES += $(MAIN)/TemplateCertificate.cpp $(MAIN)/Student.cpp
MAIN_SOURCES += $(MAIN)/Configuration.cpp
MAIN_OBJS = $(addsuffix .o, $(basename $(MAIN_SOURCES)))
MAIN_CPP = -I$(MAIN)/ -I$(NLOHMANN_JSON)/ -I$(SPDLOG)
MAIN_LDFLAGS = -lpthread

THRIFT_SOURCES = $(THRIFT_GENERATED)/$(basename $(THRIFTFILE)).cpp
THRIFT_SOURCES += $(THRIFT_GENERATED)/$(basename $(THRIFTFILE))_constants.cpp
THRIFT_SOURCES += $(THRIFT_GENERATED)/$(basename $(THRIFTFILE))_types.cpp
THRIFT_OBJS = $(addsuffix .o, $(basename $(THRIFT_SOURCES)))
THRIFT_CPP = -I$(THRIFT_GENERATED)/ -I/usr/local/include/thrift
THRIFT_LDFLAGS = -L/usr/local/lib -lthrift

SERVER_EXE = server
SERVER_SOURCES = $(SERVER)/Server.cpp
SERVER_OBJS = $(addsuffix .o, $(basename $(SERVER_SOURCES)))
SERVER_CPP = -I$(CXXOPTS)
SERVER_CPP += $(THRIFT_CPP) $(MAIN_CPP)
SERVER_LDFLAGS = $(THRIFT_LDFLAGS) $(MAIN_LDFLAGS)

CLIENT_EXE = client
CLIENT_SOURCES = $(CLIENT)/Client.cpp
CLIENT_OBJS = $(addsuffix .o, $(basename $(CLIENT_SOURCES)))
CLIENT_CPP = -I$(CXXOPTS)
CLIENT_CPP += $(THRIFT_CPP) $(MAIN_CPP)
CLIENT_LDFLAGS = $(THRIFT_LDFLAGS) $(MAIN_LDFLAGS)

LOCAL_EXE = local
LOCAL_SOURCES = $(LOCAL)/Launcher.cpp
LOCAL_OBJS = $(addsuffix .o, $(basename $(LOCAL_SOURCES)))
LOCAL_CPP = -I$(CXXOPTS)
LOCAL_CPP += $(MAIN_CPP)
LOCAL_LDFLAGS = $(MAIN_LDFLAGS)

#Tests
GENERATOR_TEST_EXE = generatorTest
#GENERATOR_TEST_SOURCES = $(GENERATOR_TEST)/RunGeneratorTests.cpp
GENERATOR_TEST_SOURCES += $(GENERATOR_TEST)/Certificate_Test.cpp
GENERATOR_TEST_SOURCES += $(GENERATOR_TEST)/Configuration_Test.cpp
GENERATOR_TEST_OBJS = $(addsuffix .o, $(basename $(GENERATOR_TEST_SOURCES)))
GENERATOR_TEST_CPP = -I$(MAIN)
GENERATOR_TEST_LDFLAGS = -lgtest -lgtest_main

#Build rules
all: docker

$(MAIN_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(MAIN_CPP) -c -o $@ $<
	
$(SERVER_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(SERVER_CPP) -c -o $@ $<
	
$(CLIENT_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(CLIENT_CPP) -c -o $@ $<
	
$(LOCAL_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(LOCAL_CPP) -c -o $@ $<
	
$(THRIFT_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(THRIFT_CPP) -c -o $@ $<
	
$(GENERATOR_TEST_OBJS): %.o : %.cpp
	$(CPP) $(CPPFLAGS) $(GENERATOR_TEST_CPP) -c -o $@ $<

$(SERVER_EXE): $(OUTPUT)/$(SERVER_EXE)

$(CLIENT_EXE): $(OUTPUT)/$(CLIENT_EXE)

$(LOCAL_EXE): $(OUTPUT)/$(LOCAL_EXE)

$(GENERATOR_TEST_EXE): $(OUTPUT)/$(GENERATOR_TEST_EXE)

$(OUTPUT)/$(SERVER_EXE): $(SERVER_OBJS) $(MAIN_OBJS) $(THRIFT_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(SERVER_LDFLAGS)
	
$(OUTPUT)/$(CLIENT_EXE): $(CLIENT_OBJS) $(MAIN_OBJS) $(THRIFT_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(CLIENT_LDFLAGS)
	
$(OUTPUT)/$(LOCAL_EXE): $(LOCAL_OBJS) $(MAIN_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(LOCAL_LDFLAGS)
	
$(OUTPUT)/$(GENERATOR_TEST_EXE): $(MAIN_OBJS) $(GENERATOR_TEST_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(GENERATOR_TEST_LDFLAGS) $(MAIN_LDFLAGS)

clean:
	rm -f $(LOCAL_OBJS) $(MAIN_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) $(THRIFT_OBJS)
	
distclean: clean
	rm -rf $(OUTPUT)
	
latexclean:
	rm -rf *.pdf *.aux *.log */*.pdf */*.aux */*.log output working

thrift:
	rm -f $(THRIFT_GENERATED)/*
	mkdir -p $(THRIFT_GENERATED)
	$(THRIFT) --gen cpp -out $(THRIFT_GENERATED) $(RESOURCES)/$(THRIFTFILE)
	
doku: 
	- doxygen Doxyfile

format: $(addprefix format-, $(wildcard $(MAIN)/*.*pp) $(wildcard $(MAIN)/*/*.*pp)))

format-$(MAIN)%.cpp: $(MAIN)%.cpp
	clang-format $< -style="{BasedOnStyle: webkit, IndentWidth: 4, TabWidth: 4, UseTab: ForContinuationAndIndentation}" > $<_2
	mv $<_2 $<
	rm -f $<_2

format-$(MAIN)%.hpp: $(MAIN)%.hpp
	clang-format $< -style="{BasedOnStyle: webkit, IndentWidth: 4, TabWidth: 4, UseTab: ForContinuationAndIndentation}" > $<_2
	mv $<_2 $<
	rm -f $<_2

format-$(MAIN)%.h: $(MAIN)%.h
	clang-format $< -style="{BasedOnStyle: webkit, IndentWidth: 4, TabWidth: 4, UseTab: ForContinuationAndIndentation}" > $<_2
	mv $<_2 $<
	rm -f $<_2

docker:
	docker build --tag=certgen .
	docker build --tag=alpinexetex res/
