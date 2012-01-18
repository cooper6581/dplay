#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "common.h"
#include "mixer.h"

int main(int argc, char **argv)
{
  struct Module *m;
  m = load_module(argv[1]);
  init_mixer(m, atoi(argv[2]));
  //play_sample(m,0);
  return 0;
}
