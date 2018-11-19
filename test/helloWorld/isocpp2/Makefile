CXX = g++
CXXFLAGS = -g -O2 -Wall -Wextra -std=c++11

TARGET_PUB = pub
SRCS_PUB = HelloWorldPub.cpp util.cpp
OBJS_PUB = ${SRCS_PUB:.cpp=.o}

TARGET_SUB = sub
SRCS_SUB = HelloWorldSub.cpp util.cpp
OBJS_SUB = ${SRCS_SUB:.cpp=.o}

IDL = HelloWorldData.idl
IDLGD = gen

OSPL_CPP_INCLUDES = -I$(OSPL_HOME)/include/dcps/C++
INCLUDES = $(OSPL_CPP_INCLUDES)/isocpp2

LIBS = -L$(OSPL_HOME)/lib -ldcpsisocpp2 -lddskernel -L$(shell pwd) -lHelloWorldData

all : pre-build main-build

pre-build:
	@echo PRE
	@cd gen;make

post-build:
	@echo POST

main-build: $(TARGET_PUB) $(TARGET_SUB)

clean:
	@echo $@...
	@cd gen;make clean
	rm -rf *.o *.gch *.so *.log $(TARGET_PUB) $(TARGET_SUB)

$(TARGET_PUB) : $(OBJS_PUB)
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

$(TARGET_SUB) : $(OBJS_SUB)
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : $(IDL) %.cpp
	@echo $@:$^...
	$(CXX) -c $(word 2,$^) $(CXXFLAGS) $(INCLUDES)

idl:
	idlpp -l isocpp2 $(IDL) -d $(IDLGD)
