#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

static noreturn void fatal(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif  // UTIL_H
