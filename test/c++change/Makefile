all: op_cpp11 op_cppold

op_cpp11: operator.cpp
	g++ -o $@ $^ -std=c++11

op_cppold: operator.cpp
	g++ -o $@ $^

clean:
	rm -rf op_cpp11 op_cppold
