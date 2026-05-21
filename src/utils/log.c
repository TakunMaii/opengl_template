
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

void PANIC(const char* content)
{
    printf("PANIC: %s, at %s, line %d\n", content, __FILE__, __LINE__);
    exit(1);
}

void INFO(const char* content)
{
    printf("INFO: %s, at %s, line %d\n", content, __FILE__, __LINE__);
}

void WARN(const char* content)
{
    printf("WARN: %s, at %s, line %d\n", content, __FILE__, __LINE__);
}

void ERROR(const char* content)
{
    printf("ERROR: %s, at %s, line %d\n", content, __FILE__, __LINE__);
}
