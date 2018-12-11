all: t1 t1l

t1: main1.cpp
	g++ -o $@ $^ -lpthread

t1l: main1.cpp
	g++ -o $@ $^ -D LOCK -lpthread

clean:
	rm -rf t1 t1l
