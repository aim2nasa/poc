CXX = g++

TARGET = sender listener detect

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

all : $(TARGET)
	@echo $@:$(TARGET)...

sender : sender.o FraudDetect.o ErrorCode.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

listener : listener.o Collector.o ErrorCode.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

detect : detect.o FraudDetect.o ErrorCode.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(CXXFLAGS) $(INCLUDES)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
