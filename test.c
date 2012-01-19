#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "loader.h"
#include "common.h"
#include "mixer.h"

int main(int argc, char **argv)
{
  struct Module *m;
  long long start_time;
  long long elapsed;
  long long last;
  m = load_module(argv[1]);
  init_mixer(m, atoi(argv[2]));
  start_time = get_millis();
  printf("start time: %lld\n", get_millis());
  for(;;) {
    elapsed = get_millis();
    last = elapsed - start_time;
    printf("last: %lld\n", last);
    start_time = elapsed;
    delay_millis(500);
    printf("1 second\n");
  }
  //play_sample(m,0);
  return 0;
}
