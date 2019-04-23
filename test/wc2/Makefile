CXX = g++
CXXFLAGS = -std=c++11 

TARGET = runTests
SRCS = main.cpp base.cpp emul.cpp
OBJS = ${SRCS:.cpp=.o}

GTEST_LIBRARIES = -L/usr/local/lib -lgtest -lpthread -lgmock
LIBS = $(GTEST_LIBRARIES)

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS)
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(CXXFLAGS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
