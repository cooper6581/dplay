#ifndef COMMON_H
#define COMMON_H

#define SAMPLE_RATE 44100
#define FRAMES 441

static const int freq_table[] = {
  907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
  856,850,844,838,832,826,820,814,808,802,796,791,785,779,774,768,
  808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
  762,757,752,746,741,736,730,725,720,715,709,704,699,694,689,684,
  720,715,709,704,699,694,689,684,678,674,670,665,660,655,651,646,
  678,675,670,665,660,655,651,646,640,637,632,628,623,619,614,610,
  640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
  604,601,597,592,588,584,580,575,570,567,563,559,555,551,547,543,
  570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
  538,535,532,528,524,520,516,513,508,505,502,498,495,491,487,484,
  508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
  480,477,474,470,467,463,460,457,453,450,447,444,441,437,434,431,

  453,450,447,444,441,437,434,431,428,425,422,419,416,413,410,407,
  428,425,422,419,416,413,410,407,404,401,398,395,392,390,387,384,
  404,401,398,395,392,390,387,384,381,379,376,373,370,368,365,363,
  381,379,376,373,370,368,365,363,360,357,355,352,350,347,345,342,
  360,357,355,352,350,347,345,342,339,337,335,332,330,328,325,323,
  339,337,335,332,330,328,325,323,320,318,316,314,312,309,307,305,
  320,318,316,314,312,309,307,305,302,300,298,296,294,292,290,288,
  302,300,298,296,294,292,290,288,285,284,282,280,278,276,274,272,
  285,284,282,280,278,276,274,272,269,268,266,264,262,260,258,256,
  269,268,266,264,262,260,258,256,254,253,251,249,247,245,244,242,
  254,253,251,249,247,245,244,242,240,239,237,235,233,232,230,228,
  240,238,237,235,233,232,230,228,226,225,224,222,220,219,217,216,

  226,225,223,222,220,219,217,216,214,213,211,209,208,206,205,204,
  214,212,211,209,208,206,205,203,202,201,199,198,196,195,193,192,
  202,200,199,198,196,195,193,192,190,189,188,187,185,184,183,181,
  190,189,188,187,185,184,183,181,180,179,177,176,175,174,172,171,
  180,179,177,176,175,174,172,171,170,169,167,166,165,164,163,161,
  170,169,167,166,165,164,163,161,160,159,158,157,156,155,154,152,
  160,159,158,157,156,155,154,152,151,150,149,148,147,146,145,144,
  151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,
  143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
  135,134,133,132,131,130,129,128,127,126,125,125,124,123,122,121
};


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
  int note;
  int played;
  int effect;
  int eparam;
};

// contains data which we need to actually PLAY the mod
// on the computer.  Sample buffers, channel information
// etc.
struct Player
{
  char mixer_buffer[1024];
  int offset;
  int size;
  int speed;
  int num_channels;
  int order_index;
  int row;
  int ticks;
  int p_break;
  int p_break_x;
  int p_break_y;
  // currently hardcoded to 4 to make sure I don't have to rewrite
  struct Channel channels[4];
  struct PatternData *pos;
  struct Module *module;
};

long long get_millis(void);
void delay_millis(int m);
int note_from_period(int period);

#endif
