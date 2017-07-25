#Define variables 
POC_ROOT = /home/pi/pocLinux
SOFTHSM2_ROOT = /home/pi/src/softhsm2/softhsm-2.3.0
OPENSSL_ROOT = /usr/local/ssl

#Compile options
SOFTHSM2_INCLUDES = -I$(SOFTHSM2_ROOT) -I$(SOFTHSM2_ROOT)/src/lib/pkcs11 -I$(SOFTHSM2_ROOT)/src/lib/common -I$(SOFTHSM2_ROOT)/src/lib/data_mgr -I$(SOFTHSM2_ROOT)/src/lib/crypto -I$(SOFTHSM2_ROOT)/src/lib/object_store
#SOFTHSM2_LIBS = -L/usr/local/lib/softhsm -lsofthsm2
#SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm2 -L$(SOFTHSM2_ROOT)/src/lib/.libs/main.o
#SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm2 -L$(SOFTHSM2_ROOT)/src/lib/.libs/libsofthsm_convarch.a
#SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs/libsofthsm_convarch.a -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm2
SOFTHSM2_LIBS = -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm_convarch -L$(SOFTHSM2_ROOT)/src/lib/.libs -lsofthsm2

GATEWAY_INCLUDES = -I$(POC_ROOT)/v0.1/gateway
OPENSSL_INCLUDES = -I$(OPENSSL_ROOT)/include
OPENSSL_LIBS = -L$(OPENSSL_ROOT)/lib -lcrypto

INCLUDES = $(GATEWAY_INCLUDES) $(SOFTHSM2_INCLUDES) $(OPENSSL_INCLUDES)
#LIBS = -ldl $(SOFTHSM2_LIBS) $(OPENSSL_LIBS)
#LIBS = $(SOFTHSM2_LIBS) $(OPENSSL_LIBS)
LIBS = $(SOFTHSM2_LIBS) $(OPENSSL_LIBS)

#OBJS = $(SOFTHSM2_ROOT)/src/lib/main.o $(SOFTHSM2_ROOT)/src/lib/SoftHSM.o $(SOFTHSM2_ROOT)/src/lib/access.o $(SOFTHSM2_ROOT)/src/lib/P11Attributes.o $(SOFTHSM2_ROOT)/src/lib/P11Objects.o
#OBJS = $(SOFTHSM2_ROOT)/src/lib/main.o
#$OBJS = $(SOFTHSM2_ROOT)/src/lib/.libs/main.o
OBJS = $(SOFTHSM2_ROOT)/src/lib/.libs/main.o $(SOFTHSM2_ROOT)/src/lib/.libs/SoftHSM.o $(SOFTHSM2_ROOT)/src/lib/.libs/P11Objects.o $(SOFTHSM2_ROOT)/src/lib/.libs/P11Attributes.o $(SOFTHSM2_ROOT)/src/lib/.libs/access.o
CXX = g++
CXXFLAGS = -g -O2 -Wall -Wextra -std=c++11 -fvisibility=hidden
#CXXFLAGS = -std=c++11
util.o : util.h util.cpp
#$(CXX) util.cpp util.h $(INCLUDES) $(CXXFLAGS) $(LIBS)
	$(CXX) util.cpp util.h $(OBJS) $(INCLUDES) $(CXXFLAGS) $(LIBS)
