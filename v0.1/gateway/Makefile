#Define variables 
#POC_ROOT = $(HOME)/poc
#SOFTHSM2_ROOT = $(HOME)/softhsm-2.3.0
#OPENSSL_ROOT = $(HOME)/openssl-1.0.2l
#ACE_ROOT = $(HOME)/ACE_wrappers

#Compile options
SOFTHSM2_INCLUDES = -I$(SOFTHSM2_ROOT) -I$(SOFTHSM2_ROOT)/src/lib/pkcs11 -I$(SOFTHSM2_ROOT)/src/lib/common -I$(SOFTHSM2_ROOT)/src/lib/data_mgr -I$(SOFTHSM2_ROOT)/src/lib/crypto -I$(SOFTHSM2_ROOT)/src/lib/object_store
SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm_convarch -lsofthsm2

COMMON_INCLUDES = -I../../common -I../common -I../hsmProxy -I$(POC_ROOT)/v0.1/common
GATEWAY_INCLUDES = -I$(POC_ROOT)/v0.1/gateway
OPENSSL_INCLUDES = -I$(OPENSSL_ROOT)/include
OPENSSL_LIBS = -L$(OPENSSL_ROOT) -lcrypto -lssl

ACE_INCLUDES = -I$(ACE_ROOT)
ACE_LIBS = -L$(ACE_ROOT)/lib -lACE -lACE_SSL

INCLUDES = $(COMMON_INCLUDES) $(GATEWAY_INCLUDES) $(SOFTHSM2_INCLUDES) $(OPENSSL_INCLUDES) -I. $(ACE_INCLUDES)
LIBS = $(SOFTHSM2_LIBS) $(ACE_LIBS) $(OPENSSL_LIBS) -L../hsmProxy -lhsmproxy -lpthread

CXX = g++
CXXFLAGS = -g -O2 -Wall -Wextra -std=c++11

TARGET = gateway
SRC = CCtrlProxy.cpp CGroup.cpp CGwData.cpp CSe.cpp Helper.cpp server.cpp StreamHandler.cpp
OBJS = $(SRC:.cpp=.o)

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o gateway $(OBJS) $(LIBS)

CCtrlProxy.o: CCtrlProxy.cpp CCtrlProxy.h CGwData.h Helper.h ../hsmProxy/CToken.h ../common/common.h
	$(CXX) -c CCtrlProxy.cpp $(INCLUDES) $(CXXFLAGS)

CGroup.o: CGroup.cpp CGroup.h
	$(CXX) -c CGroup.cpp $(INCLUDES) $(CXXFLAGS)

CGwData.o: CGwData.cpp CGwData.h
	$(CXX) -c CGwData.cpp $(INCLUDES) $(CXXFLAGS)

CSe.o: CSe.cpp CSe.h
	$(CXX) -c CSe.cpp $(INCLUDES) $(CXXFLAGS)

Helper.o: Helper.cpp Helper.h
	$(CXX) -c Helper.cpp $(INCLUDES) $(CXXFLAGS)

server.o: server.cpp
	$(CXX) -c server.cpp $(INCLUDES) $(CXXFLAGS)

StreamHandler.o: StreamHandler.cpp StreamHandler.h
	$(CXX) -c StreamHandler.cpp $(INCLUDES) $(CXXFLAGS)

clean:
	rm -rf *.o $(TARGET)
