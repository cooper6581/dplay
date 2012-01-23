#ifndef LOADER_H
#define LOADER_H

struct Module * load_module(char *fname);
void print_module(struct Module *m);

#endif
