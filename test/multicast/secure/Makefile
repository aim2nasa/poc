CXX = g++
CXXFLAGS = -g -ggdb -std=c++11

TARGET = pub pubs pubd pubsd sub dcol detect replay psender

INCLUDES = -I$(shell pwd)/../../gtest
CRYPTOPP_LIBRARIES = -lcryptopp
LIBS = $(CRYPTOPP_LIBRARIES) -lpthread

SRCS = sender.cpp FraudDetect.cpp ErrorCode.cp Classifier.cpp
OBJS = ${SRCS:.cpp=.o}

all : $(TARGET)
	@echo $@:$(TARGET)...

pub : sender.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

pubs : sender.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D PUBKEY_SECURITY $(INCLUDES) $(LIBS)

pubd : sender.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D SENDER_DETECT $(INCLUDES) $(LIBS)

pubsd : sender.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D PUBKEY_SECURITY -D SENDER_DETECT $(INCLUDES) $(LIBS)

sub : sub.o Collector.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

dcol : dcol.o Collector.o ErrorCode.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) -D LISTENER_DETECT $(INCLUDES) $(LIBS)

detect : detect.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

replay : replay.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

psender : packetSender.o FraudDetect.o ErrorCode.o Classifier.o
	@echo $@:$^...
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDES) $(LIBS)

%.o : %.cpp
	@echo $@:$<...
	$(CXX) -c $< $(CXXFLAGS) $(INCLUDES)

clean:
	@echo $@...
	rm -rf *.o $(TARGET)
