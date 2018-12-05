CXX = g++

TARGET = pub pubs pubd pubsd sub subd detect

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

all : $(TARGET)
	@echo $@:$(TARGET)...

pub : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

pubs : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ -D PUBKEY_SECURITY $(INCLUDES) $(LIBS)

pubd : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ -D SENDER_DETECT $(INCLUDES) $(LIBS)

pubsd : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ -D PUBKEY_SECURITY -D SENDER_DETECT $(INCLUDES) $(LIBS)

sub : listener.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

subd : listener.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ -D LISTENER_DETECT $(INCLUDES) $(LIBS)

detect : detect.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
