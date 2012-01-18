#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

int main(int argc, char **argv)
{
  load_module(argv[1]);
  return 0;
}
