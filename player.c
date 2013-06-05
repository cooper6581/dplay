#include <stdio.h>
#include <string.h>
#include "common.h"

static void update_row(struct Player *p, struct Module *m, 
                        unsigned long count);
static void update_tick(struct Player *p);
static void update_buffer(struct Player *p);
static float get_pitch(struct Sample *sample, int note);
static void get_xy(int col, int *x, int *y);

void init_player(struct Player *p, struct Module *m)
{
  p->module = m;
  p->size = 882;
  p->offset = 0;
  p->num_channels = 4;
  p->order_index = 0;
  p->speed = 6;
  p->row = 0;
  p->p_break = 0;
  p->p_break_x = 0;
  p->p_break_y = 0;
  p->ticks = p->speed;
  p->pos = &m->pattern_data[m->order[p->order_index]* m->channels * 64];
  memset(p->mixer_buffer, 0, p->size);
  for(int c = 0; c < p->num_channels; c++) {
    memset(&p->channels[c], 0, sizeof(struct Channel));
    p->channels[c].sample = NULL;
    p->channels[c].effect = -1;
  }
}

void play_module(struct Player *p, 
                  struct Module *m, unsigned long count)
{
  // check to see if our buffer has been written out
  if(p->offset >= p->size) {
    p->offset=0;
  }
  // if not, lets not waste CPU cycles
  if(p->offset!=0)
    return;
  // main play routine
  p->ticks++;
  if(p->ticks >= p->speed) {
    // check for pattern break
    if (p->p_break) {
      p->order_index++;
      p->row = p->p_break_x * 10 + p->p_break_y;
      p->pos = &m->pattern_data[m->order[p->order_index] * m->channels * 64];
      p->p_break = 0;
      p->p_break_x = 0;
      p->p_break_y = 0;
    }
    update_row(p, m, count);
    p->ticks = 0;
    p->row++;
    if(p->row < 64)
      p->pos+=4;
    else {
      p->order_index++;
      p->row=0;
      p->pos = &m->pattern_data[m->order[p->order_index] * m->channels * 64];
    }
  }
  update_tick(p);
}

static void update_buffer(struct Player *p)
{
  for(int i = 0; i < p->size; i++) {
    int temp = 0;
    for (int c = 0; c < p->num_channels; c++) {
      struct Sample *s = NULL;
      s = p->channels[c].sample;
      if(s == NULL)
        continue;
      // loop samples
      if (s->loop_length > 2) {
        if(p->channels[c].offset >= s->loop_length + s->loop_start)
          p->channels[c].offset = s->loop_start;
      }
      else {
        if (p->channels[c].offset >= s->length) {
          p->channels[c].played = 1;
        }
      }
      if (p->channels[c].played == 0 || s->loop_length > 2){
        temp+= (s->sample_data[(int)p->channels[c].offset]) *
          p->channels[c].volume;
        p->channels[c].offset+=p->channels[c].pitch;
      }
    }
    temp = temp / 4.0;
    p->mixer_buffer[i] = (char)temp;
  }

}

// TODO:  SO insanely ghetto
// This function is used to update effects that need to update each tick
static void update_tick(struct Player *p)
{
  for (int c = 0; c < p->num_channels; c++) {
    struct Channel *cn = &p->channels[c];
    int ex = 0;
    int ey = 0;
    get_xy(cn->eparam, &ex, &ey);
    // check for arp
    if (cn->effect == 0 && cn->eparam != 0) {
      if (cn->note == 0)
	break;
      switch(p->ticks % 3) {
      case 0:
	cn->pitch = get_pitch(cn->sample, cn->note);
	break;
      case 1:
	cn->pitch = get_pitch(cn->sample, cn->note + (ey * 16));
	break;
      case 2:
	cn->pitch = get_pitch(cn->sample, cn->note + (ex * 16));
	break;
      default:
	break;
      }
    }
    // Currently causing gapper like effect on 
    //        ./mod/radix-rainy_summerdays.mod
    else if (cn->effect == 10 && cn->eparam != 0) {
      if (ex > 0 || (ex > 0 && ey > 0)) {
	float res = (float) ex * (p->ticks - 1);
	float test = cn->volume * 64.0 + res;
	if (test <= 64)
	  cn->volume += res / 64.0;
	else
	  cn->volume = 1.0;
      } else {
	float res = (float) ey * (p->ticks - 1);
	float test = cn->volume * 64.0 - res;
	if (test > 0)
	  cn->volume -= res / 64.0;
	else
	  cn->volume = 0;
      }
    }
  }
  update_buffer(p);
}

static void update_row(struct Player *p, 
                        struct Module *m, unsigned long count)
{
  // current row
  struct PatternData *cr = p->pos;
  // index for each channel used for freq playback
  // calculate the freq ratio for each channel
  for(int i = 0 ; i < p->num_channels; i++) {
    // ONLY RESET PITCH IF THERE IS A PERIOD VALUE
    if(cr[i].period != 0) {
      p->channels[i].pitch = get_pitch(p->channels[i].sample, cr[i].period);
      p->channels[i].note = cr[i].period;
      // ONLY RESET SAMPLE IF THERE IS A PERIOD VALUE
      p->channels[i].offset = 0;
      p->channels[i].played = 0;
    }
    // ONLY RESET VOLUME IF THERE IS AN INSTRUMENT NUMBER
    if(cr[i].sample_number != 0) {
      // first let's set the sample
      p->channels[i].sample = &m->samples[cr[i].sample_number-1];
      p->channels[i].volume = p->channels[i].sample->volume / 64.0;
    }
    switch (cr[i].effect) {
    // These effects require processing each tick
    //////////////////////////////////////////////
    // Arpeggio
    case 0x00:
    // Volume slide
    case 0x0A:
      p->channels[i].effect = cr[i].effect;
      p->channels[i].eparam = cr[i].eparam;
      break;
     // These effects require processing each row
     ////////////////////////////////////////////
    // Set volume 
    case 0xc:
      p->channels[i].volume = cr[i].eparam / 64.0;
      break;
    // Pattern break
    case 0xd:
      p->p_break = 1;
      get_xy(cr[i].eparam, &p->p_break_x, &p->p_break_y);
      break;
    // Set speed
    case 0xe:
      printf("Effect 14: %d\n", cr[i].eparam);
      break;
    case 0xf:
      p->speed = cr[i].eparam;
      break;
    default:
      printf("Effect %d not implemented\n", cr[i].effect);
    }
  }
}

static float get_pitch(struct Sample *sample, int note)
{
  int ft = 0;
  int amiga_value;
  if(sample != NULL)
    ft = sample->fine_tune;
  amiga_value = freq_table[note + ft];
  return (7159090.5 / (amiga_value * 2.0)) / 44100.0;
}

static void get_xy(int col, int *x, int *y)
{
  *x = col >> 4;
  *y = col & 0x0f;
}
