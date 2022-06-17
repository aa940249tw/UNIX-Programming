CC = g++
CFLAGS = -Wall -Wextra -O2 -lcapstone -std=c++17

hw4: debugger.cpp
	$(CC) $< -o $@ $(CFLAGS)
run:
	./hw4 sample/hello64
clean:
	rm -f hw4