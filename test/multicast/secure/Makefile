CXX = g++
CXXFLAGS = -g -ggdb

TARGET = pub pubs pubd pubsd sub dcol detect

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

all : $(TARGET)
	@echo $@:$(TARGET)...

pub : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

pubs : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D PUBKEY_SECURITY $(INCLUDES) $(LIBS)

pubd : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D SENDER_DETECT $(INCLUDES) $(LIBS)

pubsd : sender.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D PUBKEY_SECURITY -D SENDER_DETECT $(INCLUDES) $(LIBS)

sub : sub.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

dcol : dcol.cpp Collector.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D LISTENER_DETECT $(INCLUDES) $(LIBS)

detect : detect.cpp FraudDetect.cpp ErrorCode.cpp
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
