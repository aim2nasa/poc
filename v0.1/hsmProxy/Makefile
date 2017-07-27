#Define variables 
POC_ROOT = /home/pi/pocLinux
SOFTHSM2_ROOT = /home/pi/src/softhsm2/softhsm-2.3.0
OPENSSL_ROOT = /usr/local/ssl

#Compile options
SOFTHSM2_INCLUDES = -I$(SOFTHSM2_ROOT) -I$(SOFTHSM2_ROOT)/src/lib/pkcs11 -I$(SOFTHSM2_ROOT)/src/lib/common -I$(SOFTHSM2_ROOT)/src/lib/data_mgr -I$(SOFTHSM2_ROOT)/src/lib/crypto -I$(SOFTHSM2_ROOT)/src/lib/object_store
SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm_convarch -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm2

GATEWAY_INCLUDES = -I$(POC_ROOT)/v0.1/gateway
OPENSSL_INCLUDES = -I$(OPENSSL_ROOT)/include
OPENSSL_LIBS = -L$(OPENSSL_ROOT)/lib -lcrypto

INCLUDES = $(GATEWAY_INCLUDES) $(SOFTHSM2_INCLUDES) $(OPENSSL_INCLUDES)
LIBS = $(SOFTHSM2_LIBS) $(OPENSSL_LIBS)

CXX = g++
CXXFLAGS = -g -O2 -Wall -Wextra -std=c++11 -fvisibility=hidden

TARGET = libhsmproxy.so.1
OBJS = library.o util.o

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS)
	@echo $@:$(OBJS)...
	$(CXX) -shared -Wl,-soname,$@ -o $@ $(OBJS)

library.o: ../../common/library.cpp ../../common/library.h 
	@echo $@...
	$(CXX) -fPIC -c ../../common/library.cpp $(INCLUDES) $(CXXFLAGS)

util.o : util.h util.cpp
	@echo $@...
	$(CXX) -fPIC -c util.cpp util.h $(INCLUDES) $(CXXFLAGS) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o *.gch $(TARGET) ../../common/*.gch
