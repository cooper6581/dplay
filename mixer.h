#ifndef MIXER_H
#define MIXER_H
#include <stdio.h> 
#include "common.h"
#include "portaudio.h"

void init_mixer(struct Module *m, int n);
void play_sample(struct Module *m, int n);

#endif
