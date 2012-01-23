#include <stdio.h> 
#include <string.h>
#include "portaudio.h"
#include "common.h"
#include "mixer.h"
#include "player.h"

#define SAMPLE_RATE 8363
#define FRAMES 1024

PaStream *stream;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
  int i;
  struct Player *p = (struct Player *)userData;
  struct Module *m = p->module;
  //char *data = p->mixer_buffer;
  char *out = (char *) outputBuffer;
  play_module(p, m);
  memcpy(out, p->mixer_buffer, framesPerBuffer);

  /*
  for(i = 0; i < framesPerBuffer; i++) {
    if(p->offset >= p->size) {
      return paContinue;
    }
    else
      *out++ = p->mixer_buffer[p->offset++] *.5;
  }
  */
  return paContinue;
}


void init_mixer(struct Player *p)
{
  PaStreamParameters outputParameters;
  PaStream *stream;
  PaError err;

  err = Pa_Initialize();
  if(err != paNoError) goto error;

  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device\n");
    goto error;
  }

  // until I come up with good upsampling code, 8-bit 8k mono
  outputParameters.channelCount = 1;
  outputParameters.sampleFormat = paInt8;
  outputParameters.suggestedLatency = 
    Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  err = Pa_OpenStream( &stream, NULL, &outputParameters, 
                        SAMPLE_RATE, FRAMES, paClipOff, 
                        patestCallback, p);
  if (err != paNoError) goto error;
  printf("starting stream\n");
  Pa_StartStream( stream );
  return;
//TODO: This is ghetto, fix asap
error:
  Pa_Terminate();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
}
