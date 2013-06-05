#include <stdio.h>
#include <string.h>
#include "common.h"

static void update_row(struct Player *p, struct Module *m, 
                        unsigned long count);
static void update_tick(struct Player *p);
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

// TODO:  SO insanely ghetto
static void update_tick(struct Player *p)
{
  struct PatternData *pos = p->pos;
  for(int i = 0; i < p->size; i++) {
    int temp = 0;
    for (int c = 0; c < p->num_channels; c++) {
      struct Sample *s = NULL;
      s = p->channels[c].sample;
      if(s == NULL)
        continue;
      // check for arp
      if (p->channels[c].effect == 0 && p->channels[c].eparam != 0) {
        struct Channel *cn = &p->channels[c];
        if (cn->note == 0)
          break;
        switch(p->ticks % 3) {
          case 0:
            cn->pitch = get_pitch(cn->sample, cn->note);
            break;
          case 1:
            cn->pitch = get_pitch(cn->sample, cn->note + ((cn->eparam & 0x0f) * 16));
            break;
          case 2:
            cn->pitch = get_pitch(cn->sample, cn->note + ((cn->eparam >> 4) * 16));
            break;
          default:
            break;
        }
      }
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
    if(cr[i].effect == 0xc) {
      p->channels[i].volume = cr[i].eparam / 64.0;
    }
    if(cr[i].effect == 0xf) {
      p->speed = cr[i].eparam;
    }
    if(cr[i].effect == 0xe5) {
      printf("Fucking finetune\n");
    }
    // arp
    if(cr[i].effect == 0x00 && cr[i].eparam != 0x00) {
      p->channels[i].effect = 0;
      p->channels[i].eparam = cr[i].eparam;
    }
    else if (cr[i].effect == 0 && cr[i].eparam == 0) {
      p->channels[i].effect = 0;
      p->channels[i].eparam = 0;
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
  return (7159090.5 / (amiga_value * 2)) / 44100.0;
}

static void get_xy(int col, int *x, int *y)
{
  *x = col >> 4;
  *y = col & 0x0f;
}
