#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

struct Module * load_module(char *fname);
void print_module(struct Module *m);

#endif
