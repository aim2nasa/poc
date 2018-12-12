all: counter show_counter

counter: counter.c
	gcc -o $@ $^

show_counter: show_counter.c
	gcc -o $@ $^

clean:
	rm -rf counter show_counter
