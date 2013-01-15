#define NDEBUG 1
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>

struct Genome {
	int *sequence;
	int count;
};

int get_line(char **line, size_t *size, FILE *file) {
	int bytesRead = getline(line, size, file);
	if (bytesRead > 0 && **line == '>') {
		bytesRead = get_line(line, size, file);
	}
	return bytesRead;	
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
	for (i = 13; i >= 0; i--) {
		int inter = *value >> (i * 2);
		inter &= 0x0003;	
		*(buffer + 13 - i) = fromBits(inter);
	}
}

void addToBuffer(int *sequence, char latest, long offset, int *sequenceMap, int *entries, struct Genome *genomeCounts) {
	*sequence = *sequence <<= 2;
	unsigned char *char_version = (unsigned char *)sequence;
	char_version[0] |= latest;
	*sequence &= 0x0FFFFFFF;
	if (offset >= 14) {
		if (sequenceMap[*sequence] != 0) {
			entries[offset + 1] = sequenceMap[*sequence];
		} else {
			entries[offset + 1] = -1;
		}
		sequenceMap[*sequence] = offset + 1;
	}
	// log_info("Sequence %X for %ld offset \n", *sequence, offset);

}

int parse_line(char *line, int *sequenceMap, long offset, int *sequence, int *entries, struct Genome *genomeCounts) {
	int length = strlen(line) - 1;
	int i = 0;
	for (i = 0; i < length; i++) {
		addToBuffer(sequence, toBits(&line[i]), offset + i, sequenceMap, entries, genomeCounts);
	}
	return i;
}

long getLength(char *fileName) {
	struct stat fileinfo;
	stat(fileName, &fileinfo);
	long length = (long)fileinfo.st_size;
	log_info("Filesize is : %ld\n", length);
	return length;
}

int *createEntries(char *fileName) {
	long length = getLength(fileName);
	int *entries = malloc(length * sizeof(int));
	return entries;
}

struct Genome *createGenomeCounts(char *fileName) {
	long length = getLength(fileName);
	struct Genome *genomeCounts = malloc(length * sizeof(struct Genome));
	return genomeCounts;
}

int count(int start, int *entries) {
	int count = 1;
	int current = start;
	while (entries[current] != -1) {
		current = entries[current];
		count++;
	}
	return count;
}

void write_counts(int *sequenceMap, int *entries) {
	int i = 0;
	char *buffer = malloc(sizeof(char) * 16);
	for (i = 0; i < pow(4, 14); i++) {
		if (sequenceMap[i] != 0) {
			toString(buffer, &i);
			printf("%d: %X, %s has %d entries\n", i, i, buffer, count(sequenceMap[i], entries));
		}
	}

}
int read_file(char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		check(file != NULL, "Could not open file %s", fileName);
	}
	log_info("starting\n");
	int *entries = createEntries(fileName);
	check(entries != NULL, "could not get enough memory for entries");
	struct Genome *genomeCounts = createGenomeCounts(fileName);
	check(genomeCounts != NULL, "could not get enough memory for genomeCounts");
	int *sequenceMap = malloc(sizeof(int) * pow(4, 14));
	check(sequenceMap != NULL, "could not get enough memory for sequenceMap");

	long numEntries = 0;
	size_t bufferSize = 1000;
	char *line = malloc(bufferSize * sizeof(char));
	log_info("int is %d\n", (int) sizeof(int));
	int *sequence = malloc(sizeof(int));
	while(get_line(&line, &bufferSize, file) > 0) {
		numEntries += parse_line(line, sequenceMap, numEntries, sequence, entries, genomeCounts);
	}
	
	log_info("processed: %ld\n", numEntries);
	
	fclose(file);
	write_counts(sequenceMap, entries);
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