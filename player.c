#include <stdio.h>
#include <string.h>
#include "common.h"

int freq_table[] = {
907,856,808,762,720,678,640,604,570,538,508,480,
453,428,404,381,360,339,320,302,285,269,254,240,
226,214,202,190,180,170,160,151,143,135,127,120,

900,850,802,757,715,675,636,601,567,535,505,477,
450,425,401,379,357,337,318,300,284,268,253,238,
225,212,200,189,179,169,159,150,142,134,126,119,

894,844,796,752,709,670,632,597,563,532,502,474,
447,422,398,376,355,335,316,298,282,266,251,237,
223,211,199,188,177,167,158,149,141,133,125,118,

887,838,791,746,704,665,628,592,559,528,498,470,
444,419,395,373,352,332,314,296,280,264,249,235,
222,209,198,187,176,166,157,148,140,132,125,118,

881,832,785,741,699,660,623,588,555,524,494,467,
441,416,392,370,350,330,312,294,278,262,247,233,
220,208,196,185,175,165,156,147,139,131,123,117,

875,826,779,736,694,655,619,584,551,520,491,463,
437,413,390,368,347,328,309,292,276,260,245,232,
219,206,195,184,174,164,155,146,138,130,123,116,

868,820,774,730,689,651,614,580,547,516,487,460,
434,410,387,365,345,325,307,290,274,258,244,230,
217,205,193,183,172,163,154,145,137,129,122,115,

862,814,768,725,684,646,610,575,543,513,484,457,
431,407,384,363,342,323,305,288,272,256,242,228,
216,203,192,181,171,161,152,144,136,128,121,114,

856,808,762,720,678,640,604,570,538,508,480,453,
428,404,381,360,339,320,302,285,269,254,240,226,
214,202,190,180,170,160,151,143,135,127,120,113
};

static void update_row(struct Player *p, const struct Module *m, 
                        unsigned long count);
static void update_tick(struct Player *p);
static float get_pitch(struct Sample *sample, int period);

void init_player(struct Player *p, struct Module *m)
{
  p->module = m;
  p->size = 880;
  p->offset = 0;
  p->num_channels = 4;
  p->pos = &m->pattern_data[m->order[0]];
  memset(p->mixer_buffer, 0, p->size);
  for(int c = 0; c < p->num_channels; c++) {
    memset(&p->channels[c], 0, sizeof(struct Channel));
    p->channels[c].sample = NULL;
  }
}

void play_module(struct Player *p, 
                  const struct Module *m, unsigned long count)
{
  if(p->offset >= p->size)
    p->offset=0;
  if(p->offset!=0)
    return;
  static int ticks;
  if(ticks == 0) {
    update_row(p, m, count);
    p->pos+=4;
    ticks = 0;
  }
  update_tick(p);
  ticks++;
  if(ticks == 6)
    ticks = 0;
}

static void update_tick(struct Player *p)
{
  for(int i = 0; i < p->size; i++) {
    int temp = 0;
    for (int c = 0; c < p->num_channels; c++) {
      struct Sample *s = NULL;
      s = p->channels[c].sample;
      if(s == NULL)
        continue;
      if (p->channels[c].offset >= s->length) {
        p->channels[c].played = 1;
        //p->channels[c].offset = 0;
      }
      // loop samples
      if (s->loop_length > 2 && p->channels[c].played == 1) {
        if(p->channels[c].offset >= s->loop_length + s->loop_start)
          p->channels[c].offset = s->loop_start;
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
      p->channels[i].pitch = get_pitch(p->channels[i].sample, cr[i].period);
      //p->channels[i].pitch = (7159090.5 / (cr[i].period * 2)) / 22000.0;
      /*
      if(p->channels[i].sample != NULL) {
        if(p->channels[i].sample->fine_tune == -3)
          p->channels[i].pitch = (7159090.5 / (cr[i].period * 2)) / 12000.0;
      }
      */
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
  }
}

static float get_pitch(struct Sample *sample, int period)
{
  int amiga_value = 0;
  int i;
  for(i = 8; i < 288; i++) {
    if(freq_table[i] == period)
      break;
  }
  if(sample == NULL)
    amiga_value = freq_table[i];
  else {
      amiga_value = freq_table[i];
  }
  return (7159090.5 / (amiga_value * 2)) / SAMPLE_RATE;
}

