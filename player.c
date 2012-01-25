#include <stdio.h>
#include <string.h>
#include "common.h"

static void update_row(struct Player *p, const struct Module *m, 
                        unsigned long count);
static void update_tick(struct Player *p);
static float get_pitch(struct Sample *sample, int period, int step);

void init_player(struct Player *p, struct Module *m)
{
  p->module = m;
  p->size = 440;
  p->offset = 0;
  p->num_channels = 4;
  p->order_index = 0;
  p->speed = 6;
  p->row = 0;
  p->ticks = 0;
  p->pos = &m->pattern_data[m->order[p->order_index]* m->channels * 64];
  memset(p->mixer_buffer, 0, p->size);
  for(int c = 0; c < p->num_channels; c++) {
    memset(&p->channels[c], 0, sizeof(struct Channel));
    p->channels[c].sample = NULL;
    p->channels[c].effect = -1;
  }
}

void play_module(struct Player *p, 
                  const struct Module *m, unsigned long count)
{
  if(p->offset >= p->size)
    p->offset=0;
  if(p->offset!=0)
    return;
  update_tick(p);
  if(p->ticks == 0) {
    update_row(p, m, count);
    p->row++;
    if(p->row < 64)
      p->pos+=4;
    else {
      p->order_index++;
      p->row=0;
      p->pos = &m->pattern_data[m->order[p->order_index] * m->channels * 64];
    }
  }
  p->ticks++;
  if(p->ticks == p->speed)
    p->ticks = 0;
}

// TODO:  SO insanely ghetto
static void update_tick(struct Player *p)
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

static void update_row(struct Player *p, 
                        const struct Module *m, unsigned long count)
{
  // current row
  struct PatternData *cr = p->pos;
  // index for each channel used for freq playback
  // calculate the freq ratio for each channel
  for(int i = 0 ; i < p->num_channels; i++) {
    // ONLY RESET PITCH IF THERE IS A PERIOD VALUE
    if(cr[i].period != 0) {
      p->channels[i].pitch = get_pitch(p->channels[i].sample, cr[i].period, 0);
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
      printf("possible BPM change: %d\n", cr[i].eparam);
      p->speed = cr[i].eparam;
    }
    if(cr[i].effect == 0xe5) {
      printf("Fucking finetune\n");
    }
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

static float get_pitch(struct Sample *sample, int period, int step)
{
  float amiga_value = 0;
  int i;
  for(i = 8; i < 576; i+=16) {
    if(period > freq_table[i] - 2 && period < freq_table[i] + 2) 
      break;
  }
  // TODO:  This is retarded, fix this
  if(sample != NULL) {
    int ft = sample->fine_tune;
    if(ft == 0)
      amiga_value = freq_table[i];
    else {
      if(ft != 0) {
        amiga_value = freq_table[i+ft];
      }
    }
  } else {
    amiga_value = freq_table[i];
  }
  return (7159090.5 / (amiga_value * 2)) / 22000.0;
}

