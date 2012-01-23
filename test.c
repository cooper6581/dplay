#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "loader.h"
#include "common.h"
#include "mixer.h"
#include "player.h"

void play_sample(const struct Sample *s, 
    struct Player *p, int n, struct Module *m)
{
  float j = 0;
  for(int i = 0; i < p->size; i++) {
    if(j < s->length) {
      p->mixer_buffer[i] = s->sample_data[(int)j];

      switch(n) {
        case 0:
          j += 1;
          break;
        case 1:
          j += 1.12336;
          break;
        case 2:
          j += 1.2625375;
          break;
        case 3:
          j += 1.3375;
          break;
        default:
          break;
      }
      //p->mixer_buffer[i+1] = s->sample_data[i];
    }
    else
      p->mixer_buffer[i] = 0;
  }
}

int main(int argc, char **argv)
{
  struct Module *m;
  struct Player p;
  long long start_time;
  long long elapsed;
  long long last;
  m = load_module(argv[1]);
  p.size = 10000;
  p.pos = m->pattern_data;
  memset(p.mixer_buffer, 0, p.size);
  init_mixer(&p);
  for(;;) {
    start_time = get_millis();
    memset(p.mixer_buffer, 0, p.size);
    play_module(&p, m);
    last = get_millis();
    elapsed = last - start_time;
    start_time = elapsed;
    delay_millis(20 - elapsed);
  }
  printf("1 second\n");
  //play_sample(m,0);
  return 0;
}
