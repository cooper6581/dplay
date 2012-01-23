#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "loader.h"

static char * load_file(char *fname);
static void load_samples(struct Module *module, char *buffer);
static void load_order(struct Module *module, char *buffer);
static int load_patterns(struct Module *module, char *buffer);
static void load_sample_data(struct Module *module, char *buffer, int soffset);
static int sbyte_to_value(const char *b);
static void print_pattern(struct Module *module, int n);

struct Module * load_module(char *fname)
{
  int soffset = 0;
  char *buffer = NULL;
  char sig[5];
  struct Module *module;
  module = malloc(sizeof(struct Module));

  buffer = load_file(fname);
  if (buffer == NULL) {
    fprintf(stderr,"Problem loading file, exiting\n");
    if (buffer != NULL)
      free(buffer);
    return NULL;
  }
  // check if it's a valid MOD
  memcpy(sig,&buffer[1080],4);
  sig[4] = '\0';
  if (strncmp(sig,"M.K.",4) == 0) {
    module->channels = 4;
  }
  else if (strncmp(sig,"6CHN",4) == 0) {
    module->channels = 6;
  }
  else if (strncmp(sig,"8CHN",4) == 0) {
    module->channels = 8;
  }
  else {
    fprintf(stderr,"Not a valid mod file, missing signature\n");
    free(buffer);
    return NULL;
  }

  // read in the module name
  strncpy(module->name,buffer,20);
  module->name[20] = '\0';

  // load the sample information
  load_samples(module, buffer);
  // load the order information
  load_order(module, buffer);
  // load the pattern information
  soffset = load_patterns(module, buffer);
  // load the sample data information
  load_sample_data(module, buffer, soffset);

  print_module(module);

  return module;
}

void print_module(struct Module *m)
{
  printf("Name: %s\n", m->name);
  printf("Channels: %02d\n", m->channels);
  printf("Sample Information\n");
  printf("--------------------------------------\n");
  for(int i = 0; i < 31; i++) {
    if (m->samples[i].length == 0)
      continue;
    printf("Sample %02d Name: %s\n", i+1, m->samples[i].name);
    printf("Sample %02d Length: %d\n", i+1, m->samples[i].length);
    printf("Sample %02d Fine Tune: %d\n", i+1, m->samples[i].fine_tune);
    printf("Sample %02d Volume: %d\n", i+1, m->samples[i].volume);
    printf("Sample %02d Loop Start: %d\n", i+1, m->samples[i].loop_start);
    printf("Sample %02d Loop Length: %d\n", i+1, m->samples[i].loop_length);
  }
  printf("\nOrder information\n");
  printf("--------------------------------------\n");
  printf("Number of Patterns: %d\n", m->number_of_patterns);
  printf("Song Length: %d\n", m->song_length);
  printf("Order Data: ");
  for(int i = 0; i < m->song_length; i++) {
    printf("%03d ", m->order[i]);
  }
  printf("\n\n");
  print_pattern(m, 0);
}

static void load_sample_data(struct Module *module, char *buffer, int soffset)
{
  struct Sample *s;
  int offset = soffset;
  for(int i = 0; i < 31; i++) {
    s = &module->samples[i];
    // allocate memory
    s->sample_data = malloc(sizeof(char) * s->length);
    if(s->sample_data == NULL)
      fprintf(stderr,"Unable to allocate memory for sample data\n");
    memcpy(s->sample_data, &buffer[offset], s->length);
    offset += s->length;
  }
}

static void print_pattern(struct Module *module, int n)
{
  struct PatternData *pos;
  pos = &module->pattern_data[n * module->channels * 64];
  for(int i = 0; i < 64; i++) {
    for(int c = 0 ; c < 4; c++) {
      printf(" %03d %03d %03d %03d | ", 
          pos->period, pos->sample_number, pos->effect, pos->eparam);
      pos++;
    }
    printf("\n");
  }
  printf("\n");
}


static int load_patterns(struct Module *module, char *buffer)
{
  int offset = 1084;
  struct PatternData *pos = NULL;
  module->pattern_data = malloc(sizeof(struct PatternData) * 
      module->channels * 64 * (module->number_of_patterns + 1));
  if (module->pattern_data == NULL)
    fprintf(stderr,"Unable to allocate memory for pattern data\n");
  pos = module->pattern_data;

  // cross fingers...
  for(int i = 0; i < module->number_of_patterns + 1; i++) {
    // CHECK:  Is it expensive to do this multiplication each iteration?
    for(int j = 0; j < 64 * module->channels; j++) {
      // reading each line in the pattern
      pos->sample_number = ((unsigned char)buffer[offset] & 0x0F0) + 
        ((unsigned char)buffer[offset+2] >> 4);
      pos->period = (((unsigned char)buffer[offset] & 0x0F)<<8) + 
        (unsigned char)buffer[offset+1];
      pos->effect = (unsigned char)buffer[offset+2] & 0x0F;
      pos->eparam = (unsigned char)buffer[offset+3];
      pos++;
      offset+=4;
    }
  }
  return offset;
}

static void load_order(struct Module *module, char *buffer)
{
  // this should be the order offset
  int offset = 950;
  int number_of_patterns = 0;
  module->song_length = buffer[offset];
  // skip unused byte
  offset += 2;
  for(int i = 0; i < 128; i++) {
    module->order[i] = buffer[offset + i];
    if(buffer[offset + i] > number_of_patterns)
      number_of_patterns = buffer[offset+i];
  }
  module->number_of_patterns = number_of_patterns;
}

static void load_samples(struct Module *module, char *buffer)
{
  struct Sample *s;
  //unsigned char stmp[2];
  int offset = 20;
  
  for(int i = 0; i < 31; i++) {
    s = &(module->samples[i]);
    // sample name
    memcpy(s->name,&buffer[offset],22);
    // trailing string term
    s->name[22] = '\0';
    // sample length
    //memcpy(stmp,&buffer[offset+22],2);
    s->length = sbyte_to_value(&buffer[offset+22]);
    // fine tune
    memcpy(&s->fine_tune,&buffer[offset+24],1);
    if(s->fine_tune > 7)
      s->fine_tune -= 16;
    // volume
    memcpy(&s->volume,&buffer[offset+25],1);
    // loop start
    s->loop_start = sbyte_to_value(&buffer[offset+26]);
    // loop length
    s->loop_length = sbyte_to_value(&buffer[offset+28]);
    offset += 30;
  }
  printf("Offset: %d\n", offset);
}

static int sbyte_to_value(const char *b)
{
  return ((unsigned char)b[0] * 0x100 + (unsigned char)b[1]) * 2;
}

static char * load_file(char *fname)
{
  char *buffer = NULL;
  FILE *fh;
  long fsize = 0;
  
  fh = fopen(fname, "rb");
  if (fh == NULL)
  {
    fprintf(stderr,"Unable to open file %s\n", fname);
    return NULL;
  }

  fseek(fh,0, SEEK_END);
  fsize = ftell(fh);
  rewind(fh);

  buffer = malloc(sizeof(char) * fsize);
  if (buffer == NULL)
  {
    fprintf(stderr,"Unable to allocate memory\n");
    return NULL;
  }
  fread(buffer, 1, fsize, fh);
  fclose(fh);
  printf("Loaded %s into memory (%ld bytes)\n", fname, fsize);
  return buffer;
} 
