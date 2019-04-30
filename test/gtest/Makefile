CXX = g++
CXXFLAGS = -std=c++11

ifeq ($(shell echo $(TEST_DEBUG)|tr [a-z] [A-Z]),YES)
	CXXFLAGS += -D DEBUG 
endif
INCLUDES = -I$(shell pwd)

TARGET = runTests
SRCS = main.cpp base.cpp key.cpp hacking.cpp encoding.cpp entity/keyStore.h entity/node.h
OBJS = ${SRCS:.cpp=.o}

GTEST_LIBRARIES = -L/usr/local/lib -lgtest -lpthread
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(GTEST_LIBRARIES) $(CRYPTOPP_LIBRARIES)

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS)
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(CXXFLAGS) $(INCLUDES)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
