#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


int main(){
	char *cwd = getcwd(NULL, 0);

	if (cwd == NULL){
		fprintf(stderr, "Ошибка");
		return 1;
	}
	printf("%s\n", cwd);
	free(cwd);
	return 0;
}
