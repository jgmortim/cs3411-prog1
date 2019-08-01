#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

/**
 * customsort.c
 *
 * Created: 1/30/19
 * Modified: 2/3/19
 * Author: John Mortimore
 */

void sortWords(char* A);
void sortLines(char** A, int numLines);
void printResult(char** A, int numLines, int fd);
void quicksort(char* A[], int lo, int hi);
int partition(char* A[], int lo, int hi);
void swap(char* A[], int a, int b);
int deleteDupLines(char** A, int numLines);
int compare(char* str1, char* str2);
int len(char str[]);


int main(int argc, char** argv){
	int alloSizeInput=5;
	int alloSizeLines=1;
	int file=0;
	int fd=0;
	int numLines=0;
	int i=0;		/* a count for loops */
	ssize_t bytes=-1;
	char delimit[]="\n";
	char *input;	
	char *line;
	char **lines;
	char *error1="customsort <input file> [output file]\n";
	char *error2="Failed to read input file\n";

	if (argc < 2 || argc > 3) { 					/* If there is less than 2 inputs, or more than 3, */
		write(1, error1, strlen(error1));		 	/* print the proper syntax, */
		return 0; 						/* and exit. */
    	}
	input = malloc(alloSizeInput*sizeof(char*));			/* Allocate some space for input. */
	file = open(argv[1], O_RDONLY);					/* Open the provided file. */
	
	/* Error checking for input. */
	if (file == -1) {
		write(1, error2, strlen(error2));
		return 0;
	}

	/* Read in characters to input and allocate space as needed. */
	for (i=0; bytes!=0; i++) {						
		if (i >= alloSizeInput-1) {
			alloSizeInput=alloSizeInput+1;
			input = realloc(input, alloSizeInput*sizeof(char*));
		}
		bytes=read(file, &input[i], 1);
	}

	input[i]='\0'; 					/* Ensure the input is null terminated. */
	close(file);					/* Close the file. */
	lines = malloc(sizeof(char**));			/* Allocate some space lines. Lines will point to the start of each line. */

	/* Replaces the \n in input with \0. Lines points to the start of each line */
	/* Valgrind does not like strtok. I spent two hours trying to fix this, no progress was made. */
	line = strtok(input, delimit);
	while (line != NULL) {
		if (numLines >= alloSizeLines) {
			alloSizeLines=alloSizeLines+5;
			lines = realloc(lines, alloSizeLines*sizeof(char**));
		}
		lines[numLines++] = line;
		line = strtok(NULL, delimit);
	}

	/* Sort the words on each line. */
	for (i = 0; i < numLines; i++)  {
  		sortWords(lines[i]);
	}

	sortLines(lines, numLines);			/* Sort all of the lines. */
	numLines = deleteDupLines(lines, numLines);	/* delete duplicate lines and record the new number of lines. */

	/* If output specified, print results to file. Otherwise, print to stdout */
	if (argc == 3) {
		fd = open(argv[2], O_WRONLY | O_APPEND | O_CREAT, 0664);
		printResult(lines, numLines, fd);
		close(fd);
	} else {
		printResult(lines, numLines, 1);
	}

	/* Free up allocated memory and return. */
	free(line);
	free(lines);
	free(input);
	return 0;
}

/* deleteDupLines
 * deletes duplicate lines in A. numLines is the number of lines in A.
 */
int deleteDupLines(char** A, int numLines) {
	int i=0;
	int j=0;
	int result;
	int deletedLines=0;
	for (i=0; i < numLines-1-deletedLines; i++) {
		if ((strcmp(A[i],A[i+1])==0) | (strlen(A[i])==0)) {
			deletedLines++;
			for (j=i; j <= (numLines-1-deletedLines); j++) {
				A[j] = A[j+1];
			}
		}
	}
	
	result=numLines-deletedLines;
	return result;
}

/* sortLines
 * sorts lines in A. numLines is the number of lines in A.
 */
void sortLines(char** A, int numLines) {
	quicksort(A, 0, numLines-1);
	return;
}

/* sortWords
 * sorts words in A.
 */
void sortWords(char* A) {
	char *word;
	char **words;
	char *line;
	char *sortedLine;
	int numWords=0;
	int j=0;
	int alloSizeWords=1;
	int alloSizeSorted=1;
	char delimit[]=" \t\r\n\v\f";

	line = strdup(A); 					/* Duplicate A so strtok doesn't mess up A. */
	words = malloc(alloSizeWords*sizeof(char**));		/* Allocate some space for words. */
	sortedLine = malloc(alloSizeSorted*sizeof(char*));	/* Allocate space. SortedLine will point to a sorted copy of A. */

	/* Replaces the spaces line input with \0. words points to the start of each word */
	word = strtok(line, delimit); 				
	while (word != NULL) {
		if (numWords >= alloSizeWords) {
			alloSizeWords=alloSizeWords+5;
			words = realloc(words, alloSizeWords*sizeof(char**));
		}
		words[numWords++] = word;
		word = strtok(NULL, delimit);
	}
	
	quicksort(words, 0, numWords-1); 			/* Sort the words. */

	/* Create a string be concatting all the words with spaces in between them. */
	/* Allocate more space as need. */
	for (j=0; j < numWords; j++) {
		if (j >= alloSizeSorted-1) {
			alloSizeSorted=alloSizeSorted+5;
			sortedLine = realloc(sortedLine, alloSizeSorted*sizeof(char*));
		}
		if (j==0) {
			strcpy(sortedLine, words[j]);
		} else {
			strcat(sortedLine, " ");
			strcat(sortedLine, words[j]);
		}
	}

	/* Repace A with the sorted string. */
	for (j=0; j < strlen(sortedLine); j++) {
		A[j]=sortedLine[j];
	}
	
	/* Free the allocated memory and return. */
	free(words);
	free(word);
	free(line);
	free(sortedLine);
	return;
}

/* printResult
 * prints the lines in A. numLines is the number of lines in A.
 * fd is the file descriptor for output. 
 */
void printResult(char** A, int numLines, int fd) {
	int i;
	for (i = 0; i < numLines; i++)  {
		write(fd, A[i], strlen(A[i]));
		write(fd, "\n", 1);
	}
	return;
}

/* quicksort
 * an implementation of quicksort.
 */
void quicksort(char** A, int lo, int hi) {
	int p = 0;
	if (lo < hi) {
		p = partition(A, lo, hi);
		quicksort(A, lo, p - 1);
		quicksort(A, p + 1, hi);
	}
	return;
}

/* partition
 * used by quicksort
 */
int partition(char** A, int lo, int hi) {
	char* pivot = A[hi];
	int i = lo;
	int j = 0;
	for (j = lo; j < hi; j++) {
		if (strcmp(A[j], pivot) < 0) {
			swap(A, i, j);
			i++;
		}
	}
	swap(A, i, hi);
	return i;
}

/* swap
 * swaps A[a] with A[b]
 */
void swap(char** A, int a, int b) {
	void* temp = A[a];
	A[a] = A[b];
	A[b] = temp;
	return;
}
