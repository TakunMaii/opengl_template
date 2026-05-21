#include "file.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned char* read_file_impl(const char* file_path, size_t* size_out)
{
    FILE *fp = fopen(file_path, "rb");
    if (!fp) PANIC("unable to open file");

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    unsigned char* buffer = (unsigned char *)malloc((size_t)size + 1);
    if (!buffer) PANIC("unable to alloc memory");

    if (fread(buffer, 1, (size_t)size, fp) != (size_t)size)
    {
        fclose(fp);
        free(buffer);
        PANIC("unable to read file");
    }
    buffer[size] = 0;

    fclose(fp);
    if (size_out != NULL)
    {
        *size_out = (size_t)size;
    }
    return buffer;
}

char* read_entire_file(const char* file_path)
{
    return (char*)read_file_impl(file_path, NULL);
}

unsigned char* read_entire_binary_file(const char* file_path, size_t* size_out)
{
    return read_file_impl(file_path, size_out);
}
