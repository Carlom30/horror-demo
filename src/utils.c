#include "utils.h"

#include <stdio.h>

int read_file_str(const char *path, char **dst)
{
	FILE *fd = fopen(path, "r");
	fseek(fd, 0, SEEK_END);
	int len = ftell(fd);
	rewind(fd);
	(*dst) = malloc(len + 1);
	fread(*dst, 1, len, fd);
	(*dst)[len] = '\0';
	fclose(fd);
	return NOERR;
}
