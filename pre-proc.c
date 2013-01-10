#define NDEBUG 1
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>

struct Counts {
	char base;
	int count;
};

int get_line(char **line, size_t *size, FILE *file) {
	int bytesRead = getline(line, size, file);
	if (bytesRead > 0 && **line == '>') {
		bytesRead = get_line(line, size, file);
	}
	return bytesRead;	
}

void process_character(char *position, struct Counts *counts) {
	counts->base = *position;
	counts->count = 0;
}

char toBits(char *input) {
	switch (*input) {
		case 'A' :
			return 0;
		case 'C' :
			return 1;
		case 'G':
		 	return 2;
		case 'T':
			return 3;
	}
	return 0;
}

char fromBits(int value) {
	switch(value) {
		case 0:
			return 'A';
		case 1:
			return 'C';
		case 2:
			return 'G';
		case 3:
			return 'T';
	}
	return 'X';
}

void toString(char *buffer, int *value) {
	int i = 0;
	for (i = 15; i > 0; i--) {
		int inter = *value >> (i);
		inter &= 0x0003;	
		*(buffer + 15 - i) = fromBits(inter);
	}
}

void addToBuffer(int *sequence, char latest, long offset) {
	*sequence = *sequence <<= 2;
	unsigned char *char_version = (unsigned char *)sequence;
	char_version[0] |= latest;
	*sequence &= 0x7FFF;
}

int parse_line(char *line, int *counts, long offset, int *sequence) {
	int length = strlen(line) - 1;
	int i = 0;
	for (i = 0; i < length; i++) {
		addToBuffer(sequence, toBits(&line[i]), offset);
		counts[*sequence] += 1;
	}
	return i;
}

struct Counts *createCounts(char *fileName) {
	struct stat fileinfo;
	stat(fileName, &fileinfo);
	long length = (long)fileinfo.st_size;
	log_info("Filesize is : %ld\n", length);
	struct Counts *counts = malloc(length * sizeof(struct Counts));
	return counts;
}

int read_file(char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		check(file != NULL, "Could not open file %s", fileName);
	}
	log_info("starting\n");
	int *counts = malloc(sizeof(int) * pow(4, 14));
	check(counts != NULL, "could not get enough memory for counts");

	long numEntries = 0;
	size_t bufferSize = 1000;
	char *line = malloc(bufferSize * sizeof(char));
	log_info("int is %d\n", (int) sizeof(int));
	int *sequence = malloc(sizeof(int));
	while(get_line(&line, &bufferSize, file) > 0) {
		numEntries += parse_line(line, counts, numEntries, sequence);
	}
	
	log_info("processed: %ld\n", numEntries);
	
	fclose(file);
	int i = 0;
	for (i = 0; i < pow(4, 14); i++) {
		if (counts[i] > 0) {
			char *buffer = malloc(sizeof(char) * 16);
//			log_info("%X has %d entries\n", i, counts[i]);
			toString(buffer, &i);
			log_info("%s has %d entries\n", buffer, counts[i]);
		}
	}
	return 0;

	error:
		return -1; 
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		log_err("filename not provided");
		return 1;
	}
	return read_file(argv[1]);
}