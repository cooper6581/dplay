#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "loader.h"
#include "common.h"
#include "mixer.h"
#include "player.h"

int main(int argc, char **argv)
{
  struct Module *m;
  struct Player p;
  m = load_module(argv[1]);
  init_player(&p, m);
  init_mixer(&p);
  for(;;) {
    delay_millis(20);
    continue;
  }
  printf("1 second\n");
  return 0;
}
