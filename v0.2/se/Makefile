#Define variables 

#set these variables in shell like, POC_ROOT=/home/pi/poc
#POC_ROOT = /home/pi/poc
#SOFTHSM2_ROOT = /home/pi/src/softhsm2/softhsm-2.3.0
#OPENSSL_ROOT = /usr/local/ssl

#Compile options
SOFTHSM2_INCLUDES = -I$(SOFTHSM2_ROOT) -I$(SOFTHSM2_ROOT)/src/lib/pkcs11 -I$(SOFTHSM2_ROOT)/src/lib/common -I$(SOFTHSM2_ROOT)/src/lib/data_mgr -I$(SOFTHSM2_ROOT)/src/lib/crypto -I$(SOFTHSM2_ROOT)/src/lib/object_store
SOFTHSM2_LIBS = -L/usr/local/lib/softhsm -lsofthsm2 -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm_convarch

COMMON_INCLUDES = -I$(POC_ROOT)/common -I$(POC_ROOT)/v0.1/common
GATEWAY_INCLUDES = -I$(POC_ROOT)/v0.1/gateway
OPENSSL_INCLUDES = -I$(OPENSSL_ROOT)/include
HSMPROXY_INCLUDES = -I$(POC_ROOT)/v0.1/hsmProxy
ACE_INCLUDES = -I/usr/local/include

OPENSSL_LIBS = -L$(OPENSSL_ROOT)/lib -lcrypto -lssl
HSMPROXY_LIBS = -L$(POC_ROOT)/v0.1/hsmProxy -lhsmproxy
ACE_LIBS = -L/usr/local/lib -lACE -lACE_SSL

INCLUDES = $(COMMON_INCLUDES) $(GATEWAY_INCLUDES) $(SOFTHSM2_INCLUDES) $(OPENSSL_INCLUDES) $(HSMPROXY_INCLUDES)
LIBS = $(HSMPROXY_LIBS) $(SOFTHSM2_LIBS) $(OPENSSL_LIBS) $(ACE_LIBS)
LIBS += -lteec -L$(TEEC_EXPORT)/lib -lokey -L$(OKEY_ROOT)

CXX = g++
CXXFLAGS = -g -O2 -Wall -Wextra -std=c++11 -fvisibility=hidden -D USE_OPTEE
CXXFLAGS += -I$(TEEC_EXPORT)/include -I$(OKEY_ROOT)

TARGET = se
OBJS = main.o

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS)
	@echo $@:$(OBJS)...
	$(CXX) -o $@ $(OBJS) $(INCLUDES) $(CXXFLAGS) $(LIBS)

main.o : main.cpp
	@echo $@...
	$(CXX) -c main.cpp $(INCLUDES) $(CXXFLAGS) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o *.gch $(TARGET)
