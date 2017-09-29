default: build



build: main.c
	gcc main.c -o main -Wall -Werror -pedantic -g

run: build
	./main

clean:
	rm main main.o

