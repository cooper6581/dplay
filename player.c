#include <stdio.h>
#include "common.h"


static void update_row(struct Player *p, const struct Module *m);

void play_module(struct Player *p, const struct Module *m)
{
  static int tick;
  tick++;
  if(tick >= 5) {
    update_row(p, m);
    tick = 0;
    p->pos+=4;
  }
}

static void update_row(struct Player *p, const struct Module *m)
{
  p->offset = 0;
  // current row
  struct PatternData *cr = p->pos;
  int tmp;
  // index for each channel used for freq playback
  static float fa, fb, fc, fd;
  float ia, ib, ic, id = 0;
  // calculate the frequencies for each channel
  if(cr[0].period != 0)
    fa = (7159090.5 / (cr[0].period * 2)) / 8363.424;
  if(cr[1].period != 0)
    fb = (7159090.5 / (cr[1].period * 2)) / 8363.424;
  if(cr[2].period != 0)
    fc = (7159090.5 / (cr[2].period * 2)) / 8363.424;
  if(cr[3].period != 0)
    fd = (7159090.5 / (cr[3].period * 2)) / 8363.424;

  for(int i = 0; i < p->size; i++) {
    char va, vb, vc, vd = 0;
    if(cr[0].sample_number != 0) {
      if(ia <= m->samples[cr[0].sample_number-1].length)
        va = m->samples[cr[0].sample_number-1].sample_data[(int)ia];
    }
    if(cr[1].sample_number != 0) {
      if(ib <= m->samples[cr[1].sample_number].length)
        vb = m->samples[cr[1].sample_number].sample_data[(int)ib];
    }
    if(cr[2].sample_number != 2) {
      if(ic <= m->samples[cr[2].sample_number].length)
        vc = m->samples[cr[2].sample_number].sample_data[(int)ic];
    }
    if(cr[3].sample_number != 3) {
      if(id <= m->samples[cr[3].sample_number].length)
        vd = m->samples[cr[3].sample_number].sample_data[(int)id];
    }

    //tmp = (va + vb + vc + vd) / 4; 
    tmp = va;
    ia += fa;
    ib += fb;
    ic += fc;
    id += fd;
    p->mixer_buffer[i] = tmp;
  }
  p->offset = 0;

}
