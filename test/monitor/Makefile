all: mon mond

mon: main.cpp
	g++ -o $@ $^ -std=c++11 -lpthread

mond: main_detail.cpp
	g++ -o $@ $^ -std=c++11 -lpthread

clean:
	rm -rf mon mond
