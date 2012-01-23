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
  p.count = 0;
  long long start_time;
  long long elapsed;
  long long last;
  m = load_module(argv[1]);
  p.module = m;
  p.size = 1024;
  p.pos = &m->pattern_data[1];
  memset(p.mixer_buffer, 0, p.size);
  init_mixer(&p);
  for(;;) {
    delay_millis(20);
    continue;
  }
  printf("1 second\n");
  printf("call back ran: %d\n",p.count);
  //play_sample(m,0);
  return 0;
}
