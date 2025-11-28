#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc,char *argv[]){
	if (argc != 3) {
		fprintf(stderr, "Usage : %s <source> <destination>\n", argv[0]);
		return 1;
	}

	char *source = argv[1];
	char *destination = argv[2];

	struct stat dest_stat;
	int dest_exists = (stat(destination, &dest_stat) == 0);
	int dest_is_dir = dest_exists && S_ISDIR(dest_stat.st_mode);

	char final_destination[1024];

	if (dest_is_dir) {
		
		const char *source_name = strrchr(source,'/');
		if (source_name == NULL) {
				source_name = source;
		} else {
			source_name++;
		}

		snprintf(final_destination, sizeof(final_destination), "%s/%s",
						destination, source_name);
	}else {
		strncpy(final_destination, destination, sizeof(final_destination));
	}


	if (rename(source, final_destination) != 0) {
		perror("mv");
		return 1;
	}

	return 0;
}
