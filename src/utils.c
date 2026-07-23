#include "utils.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int read_file_str(const char *path, char **dst)
{
	if (!path || !dst) return ERR;

	*dst = NULL;

	FILE *fd = fopen(path, "rb");
	if (!fd) return ERR;

	if (fseek(fd, 0, SEEK_END) != 0) {
		fclose(fd);
		return ERR;
	}

	long pos = ftell(fd);
	if (pos < 0) {
		fclose(fd);
		return ERR;
	}
	rewind(fd);
	size_t len = (size_t)pos;

	char *buf = (char *)malloc(len + 1);
	if (!buf) {
		fclose(fd);
		return ERR;
	}

	size_t nread = fread(buf, 1, len, fd);
	if (nread != len) {
		free(buf);
		fclose(fd);
		return ERR;
	}

	buf[len] = '\0';

	fclose(fd);
	*dst = buf;
	return NOERR;
}

int png_load(const char *path, png *dst)
{
	if (dst == NULL)
		return ERR;
	png p = {0};
	p.data = stbi_load(path, &p.w, &p.h, &p.n, 4);
	if (!p.data)
		return ERR;
	*dst = p;
	return NOERR;
}
