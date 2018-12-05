CXX = g++

TARGET = pub sub detect

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

all : $(TARGET)
	@echo $@:$(TARGET)...

pub : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

sub : listener.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

detect : detect.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
