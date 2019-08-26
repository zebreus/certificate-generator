#Paths
OUTPUT = ./out/
MAIN = ./src/
SERVER = ./src/server/
CLIENT = ./src/client/
LOCAL = ./src/local/
NLOHMANN_JSON = ./libs/json/
CXXOPTS = ./libs/cxxopts/
THRIFT_GENERATED = $(MAIN)/gen-cpp/
RESOURCES = ./res/

#Configuration
CPP  = g++
THRIFT = thrift
THRIFTFILE = CertificateGenerator.thrift
CPPFLAGS = -Wall -Wformat -Os -std=c++17

#Sourcecode and flags
MAIN_SOURCES = $(MAIN)/Certificate.cpp $(MAIN)/Batch.cpp 
MAIN_SOURCES += $(MAIN)/TemplateCertificate.cpp $(MAIN)/Student.cpp
MAIN_OBJS = $(addsuffix .o, $(basename $(MAIN_SOURCES)))
MAIN_CPP = -I$(MAIN)/ -I$(NLOHMANN_JSON)/
MAIN_LDFLAGS = 

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

#Build rules
all: $(LOCAL_EXE)

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

$(SERVER_EXE): $(OUTPUT)/$(SERVER_EXE)

$(CLIENT_EXE): $(OUTPUT)/$(CLIENT_EXE)

$(LOCAL_EXE): $(OUTPUT)/$(LOCAL_EXE)

$(OUTPUT)/$(SERVER_EXE): $(SERVER_OBJS) $(MAIN_OBJS) $(THRIFT_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(SERVER_LDFLAGS)
	
$(OUTPUT)/$(CLIENT_EXE): $(CLIENT_OBJS) $(MAIN_OBJS) $(THRIFT_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(CLIENT_LDFLAGS)
	
$(OUTPUT)/$(LOCAL_EXE): $(LOCAL_OBJS) $(MAIN_OBJS)
	mkdir -p $(OUTPUT)
	$(CXX) -o $@ $^ $(LOCAL_LDFLAGS)

clean:
	rm -f $(LOCAL_OBJS) $(MAIN_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) $(THRIFT_OBJS)

thrift:
	rm -f $(THRIFT_GENERATED)/*
	mkdir -p $(THRIFT_GENERATED)
	$(THRIFT) --gen cpp -out $(THRIFT_GENERATED) $(RESOURCES)/$(THRIFTFILE)
	
doku: 
	- doxygen Doxyfile

docker:
	docker build --tag=certgen .
