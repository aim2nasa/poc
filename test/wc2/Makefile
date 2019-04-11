CXX = g++

TARGET = runTests
SRCS = main.cpp base.cpp mcnet.cpp
OBJS = ${SRCS:.cpp=.o}

GTEST_LIBRARIES = -lgtest -lpthread
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
