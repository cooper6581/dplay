#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static void update_row(struct Player *p, struct Module *m, 
                        unsigned long count);
static void update_tick(struct Player *p);
static float get_pitch(struct Sample *sample, int note);
static void get_xy(int col, int *x, int *y);
static int get_amiga_value(struct Sample *sample, int note);
static float get_pitch_delta(struct Sample *sample, int note, int delta);
extern long rate;

void init_player(struct Player *p, struct Module *m)
{
  p->module = m;
  p->size = 512;
  p->offset = 0;
  p->num_channels = 4;
  p->order_index = 0;
  p->speed = 6;
  p->row = 0;
  p->p_break = 0;
  p->p_break_x = 0;
  p->p_break_y = 0;
  p->rate = 1000000 / 50;
  p->ticks = 0;
  p->pos = &m->pattern_data[m->order[p->order_index]* m->channels * 64];
  memset(p->mixer_buffer, 0, p->size);
  for(int c = 0; c < p->num_channels; c++) {
    memset(&p->channels[c], 0, sizeof(struct Channel));
    p->channels[c].sample = NULL;
    p->channels[c].effect = -1;
    p->channels[c].vibrato_pos = 0;
    p->channels[c].vibrato_depth = 0;
    p->channels[c].vibrato_speed = 0;
    // XXX:  this should be in the channel struct
    p->porta_speed[c] = 0;
    p->note_to_porta_to[c] = 0;
  }
}

void play_module(struct Player *p, 
                  struct Module *m, unsigned long count)
{
  // main play routine
  //p->ticks++;
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
  p->ticks++;
}

void update_buffer(struct Player *p, int framesPerBuffer)
{
  for(int i = 0; i < framesPerBuffer; i++) {
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
    temp /= 4;
    p->mixer_buffer[i] = (char)temp;
  }
}

// TODO:  SO insanely ghetto
// This function is used to update effects that need to update each tick
static void update_tick(struct Player *p)
{
  //printf("Tick: %d\n", p->ticks);
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
    // volume slide
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
    // porta up
    else if (cn->effect == 1) {
      // don't run effect for the first tick
      //if (p->ticks) {
	int b3 = note_from_period(113);
	cn->note += cn->eparam;
	if (cn->note > b3)
	  cn->note = b3;
	cn->pitch = get_pitch(cn->sample, cn->note);
	//}
    }
    // porta down
    else if (cn->effect == 2) {
      // don't run effect for the first tick
      if (p->ticks) {
	int c1 = note_from_period(856);
	cn->note -= cn->eparam;
	if (cn->note < c1)
	  cn->note = c1;
	cn->pitch = get_pitch(cn->sample, cn->note);
      }
    }
    // Porta slide
    else if (cn->effect == 3) {
      if (p->ticks == 0) {
	// we need to read the previous row
	struct PatternData *cr = p->pos - 4;
	if (cn->eparam)
	  p->porta_speed[c] = cn->eparam;
	if (cr[c].period)
	  p->note_to_porta_to[c] = cr[c].period;
	//return;
      }
      if (p->note_to_porta_to[c] > cn->note) {
	cn->note += p->porta_speed[c];
	if (cn->note >= p->note_to_porta_to[c])
	  cn->note = p->note_to_porta_to[c];
	cn->pitch = get_pitch(cn->sample, cn->note);
      }
      else if (p->note_to_porta_to[c] < cn->note) {
	cn->note -= p->porta_speed[c];
	if (cn->note <= p->note_to_porta_to[c])
	  cn->note = p->note_to_porta_to[c];
	cn->pitch = get_pitch(cn->sample, cn->note);
      }
    }
    // Vibrato
    else if (cn->effect == 4) {
      if (p->ticks == 0) {
	cn->vibrato_pos = 0;
	return;
      }
      if (ey)
	cn->vibrato_depth = ey;
      if (ex)
	cn->vibrato_speed = ex;
      int sin_value = sine_table[abs(cn->vibrato_pos)];
      int delta = (sin_value * cn->vibrato_depth) / 128;
      if (cn->vibrato_pos < 0)
	cn->pitch = get_pitch_delta(cn->sample,cn->note,delta);
      else
	cn->pitch = get_pitch_delta(cn->sample,cn->note, -delta);
      cn->vibrato_pos += cn->vibrato_speed;
      if (cn->vibrato_pos > 31)
	cn->vibrato_pos -= 63;
    }
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
    // ONLY RESET PITCH IF THERE IS A PERIOD VALUE AND PORTA NOT SET
    if(cr[i].period != 0 && cr[i].effect !=3) {
      p->channels[i].pitch = get_pitch(p->channels[i].sample, cr[i].period);
      p->channels[i].note = cr[i].period;
      // ONLY RESET SAMPLE IF THERE IS A PERIOD VALUE
      p->channels[i].offset = 0;
      p->channels[i].played = 0;
    }
    // clear the channel effects
    p->channels[i].effect = 0;
    p->channels[i].eparam = 0;

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
    // Porta up
    case 0x01:
    // Porta down
    case 0x02:
    // Porta to note
    case 0x03:
    // Vibrato
    case 0x04:
    // Volume slide
    case 0x0A:
      p->channels[i].effect = cr[i].effect;
      p->channels[i].eparam = cr[i].eparam;
      break;
     // These effects require processing each row
     ////////////////////////////////////////////
    // Set volume 
    case 0xc:
      if (cr[i].eparam > 64)
	cr[i].eparam = 64;
      p->channels[i].volume = cr[i].eparam / 64.0;
      break;
    // Pattern break
    case 0xd:
      p->p_break = 1;
      get_xy(cr[i].eparam, &p->p_break_x, &p->p_break_y);
      break;
    case 0xe:
      printf("Effect 14: %d\n", cr[i].eparam);
      break;
    // Set speed
    case 0xf: {
      int x = 0;
      int y = 0;
      get_xy(cr[i].eparam, &x, &y);
      int z = x * 16 + y;
      if (z < 32)
	p->speed = z;
      // XXX set bpm
      else {
	p->rate = bpm_to_rate(cr[i].eparam);
	printf("Set rate to %ld\n", p->rate);
      }
      break;
    }
    default:
      ;
      //printf("Effect %d not implemented\n", cr[i].effect);
    }
  }
}

static float get_pitch_delta(struct Sample *sample, int note, int delta)
{
  int amiga_value = get_amiga_value(sample, note);
  return (7159090.5 / ((amiga_value + delta) * 2.0)) / SAMPLE_RATE;
}

static float get_pitch(struct Sample *sample, int note)
{
  int amiga_value = get_amiga_value(sample, note);
  return (7159090.5 / (amiga_value * 2.0)) / SAMPLE_RATE;
}

static int get_amiga_value(struct Sample *sample, int note)
{
  int ft =0;
  if(sample != NULL)
    ft = sample->fine_tune;
  return freq_table[note + ft];
}

static void get_xy(int col, int *x, int *y)
{
  *x = col >> 4;
  *y = col & 0x0f;
}
