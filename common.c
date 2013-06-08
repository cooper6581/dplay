#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

int note_from_period(int period)
{
  for(int i = 8; i < 576; i+=16) {
    if (period == freq_table[i])
      return i;
    //if(period < freq_table[i] + 7 && period > freq_table[i] - 8)
    //  return i;
  }
  return 0;
}

long bpm_to_rate(int bpm)
{
  return 1000000 / ((bpm * 2) / 5);
}
