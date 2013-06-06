#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "loader.h"
#include "common.h"
#include "mixer.h"
#include "player.h"

int main(int argc, char **argv)
{
  long useconds;
  struct timeval start, elapsed;
  struct Module *m;
  struct Player p;
  m = load_module(argv[1]);
  init_player(&p, m);
  init_mixer(&p);
  for(;;) {
    gettimeofday(&start, NULL);
    play_module(&p, m, 1024);
    gettimeofday(&elapsed, NULL);
    useconds = elapsed.tv_usec - start.tv_usec;
    usleep(p.rate - useconds);
    continue;
  }
  return 0;
}
