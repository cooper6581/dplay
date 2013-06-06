#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

int note_from_period(int period)
{
  for(int i = 8; i < 576; i+=16) {
    if(period < freq_table[i] + 2 && period > freq_table[i] - 2)
      return i;
  }
  return 0;
}

