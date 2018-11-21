CXX = g++

TARGET = sender listener
SRCS = sender.cpp
OBJS = ${SRCS:.cpp=.o}

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES)

all : $(TARGET)
	@echo $@:$(TARGET)...

$(TARGET) : $(OBJS)
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(INCLUDES)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
