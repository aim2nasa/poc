CXX = g++
CXXFLAGS = -std=c++11 

CURDIR = $(shell pwd)
TARGET = runTests
SRCS = main.cpp baseTest.cpp emulTest.cpp SessionTest.cpp DetectTest.cpp
OBJS = ${SRCS:.cpp=.o}

INCLUDES = -I$(CURDIR)/../multicast/secure -I$(CURDIR)/../gtest
GTEST_LIBRARIES = -L/usr/local/lib -lgtest -lpthread -lgmock
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(GTEST_LIBRARIES) $(CRYPTOPP_LIBRARIES)

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS) Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(INCLUDES) $(CXXFLAGS)

Classifier.o : $(CURDIR)/../multicast/secure/Classifier.cpp
	@echo $@:$<...
	$(CXX) -c $< $(INCLUDES) $(CXXFLAGS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
