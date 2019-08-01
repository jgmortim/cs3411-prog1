CC = gcc
FLAGS = -pedantic -Wall

target: customsort.c
	$(CC) $(FLAGS) customsort.c -o customsort
clean:
	rm customsort -f *.o
