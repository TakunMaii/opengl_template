#ifndef FILE_H
#define FILE_H

#include <stddef.h>

char* read_entire_file(const char* file_path);
unsigned char* read_entire_binary_file(const char* file_path, size_t* size_out);

#endif /* FILE_H */

