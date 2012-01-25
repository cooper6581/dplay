#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

long long get_millis(void)
{
 struct timespec ts;
 clock_gettime(CLOCK_REALTIME, &ts);
 return (long long)(ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));
}

void delay_millis(int m)
{
  struct timespec ts, tr;
  ts.tv_sec = 0;
  ts.tv_nsec = m * 1000000UL;
  nanosleep(&ts, &tr);
}

/*
int note_from_period(int period)
{
  for(int i = 8; i < 
}
*/
