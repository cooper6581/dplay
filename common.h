#ifndef COMMON_H
#define COMMON_H

#define SAMPLE_RATE 44100
#define FRAMES 8

// Used to store each row of pattern data
struct PatternData
{
  unsigned char sample_number;
  int period;
  unsigned char effect;
  unsigned char eparam;
};

// Used to represent the actual samples in the MOD
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

// This struct is only used to represent the MOD file as data
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

// Contains channel data used to play the MOD
struct Channel
{
  // ratio used to calculate the pitch from the period 
  // field in the pattern
  float pitch;
  // points to where we are in the sample buffer
  float offset;
  // ratio of pattern data where 64 == 1, 32 == .5, etc.
  float volume;
  // pointer to the current sample number
  struct Sample *sample;
  int played;
};

// contains data which we need to actually PLAY the mod
// on the computer.  Sample buffers, channel information
// etc.
struct Player
{
  char mixer_buffer[1024];
  int offset;
  int size;
  int num_channels;
  // currently hardcoded to 4 to make sure I don't have to rewrite
  struct Channel channels[4];
  struct PatternData *pos;
  struct Module *module;
};

long long get_millis(void);
void delay_millis(int m);

#endif
