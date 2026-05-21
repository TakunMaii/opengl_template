#include "file.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

char* read_entire_file(const char* file_path)
{
    FILE *fp = fopen(file_path, "rb");
    if (!fp) PANIC("unable to open file");

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char* buffer = (char *)malloc(size + 1);
    if (!buffer) PANIC("unable to alloc memory");

    fread(buffer, 1, size, fp);
    buffer[size] = 0;

    fclose(fp);
    return buffer;
}
