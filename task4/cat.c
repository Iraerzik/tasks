#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_file(const char *filename, int number_lines, int *line_counter){
	FILE *file = filename ? fopen(filename, "r"): stdin;
	if (!file) {
		perror(filename);
		return;
	}

	char buffer[4096];

	while (fgets(buffer, sizeof(buffer) , file)) {
		if (number_lines) {
			printf("%6d %s", (*line_counter)++, buffer);
		} else {
			printf("%s", buffer);
		}
	}

	fclose(file);
}









int main(int argc, char *argv[]){
	int number_lines = 0;
	int file_count = 0;
	char **filenames = NULL;
	
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i],"-n") == 0) {
			number_lines = 1;
		} else {
			file_count++;
			filenames = realloc(filenames, file_count * sizeof(char *));
			filenames[file_count - 1] = argv[i];
		}
	}
	
	int line_counter = 1;

	if (file_count == 0) {
		print_file(NULL, number_lines, &line_counter);
	} else {
		for (int i = 0; i < file_count; i++) {
			print_file(filenames[i], number_lines, &line_counter);
		}
		free(filenames);
	}

	return 0;
}
