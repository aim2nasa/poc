CXX = g++

TARGET = sender listener

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES)

all : $(TARGET)
	@echo $@:$(TARGET)...

sender : sender.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

listener : listener.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(INCLUDES)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
