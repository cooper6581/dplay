#ifndef COMMON_H
#define COMMON_H

struct PatternData
{
  unsigned char sample_number;
  int period;
  unsigned char effect;
  unsigned char eparam;
};

struct Sample
{
  char name[23];
  int length;
  char fine_tune;
  unsigned char volume;
  int loop_start;
  int loop_length;
  char *sample_data;
};

struct Module
{
  unsigned char channels;
  char name[21];
  struct Sample samples[31];
  unsigned char song_length;
  unsigned char number_of_patterns;
  unsigned char order[128];
  struct PatternData *pattern_data;
};

long long get_millis(void);
void delay_millis(int m);

#endif
