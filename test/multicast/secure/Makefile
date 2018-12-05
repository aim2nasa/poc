CXX = g++

TARGET = sender listener detect

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

all : $(TARGET)
	@echo $@:$(TARGET)...

sender : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

listener : listener.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

detect : detect.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
