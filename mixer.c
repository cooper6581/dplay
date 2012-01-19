#include "common.h"
#include "mixer.h"

#define SAMPLE_RATE 8363
#define FRAMES 64

PaStream *stream;
int length;
int offset;
int stupid;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
  int i;
  char *data = (char *)userData;
  char *out = (char *) outputBuffer;

  for(i = 0; i < framesPerBuffer; i++) {
    if (offset >= length)
      *out++ = 0;
    else
        *out++ = data[offset++] *.2;
  }
  return paContinue;
}


void init_mixer(struct Module *m, int n)
{
  PaStreamParameters outputParameters;
  PaStream *stream;
  PaError err;
  char *data;

  data = m->samples[n].sample_data;
  length = m->samples[n].length;
  offset = 0;

  err = Pa_Initialize();
  if(err != paNoError) goto error;

  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device\n");
    goto error;
  }

  outputParameters.channelCount = 1;
  outputParameters.sampleFormat = paInt8;
  outputParameters.suggestedLatency = 
    Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  err = Pa_OpenStream( &stream, NULL, &outputParameters, SAMPLE_RATE, FRAMES,
      paClipOff, patestCallback, data);
  if (err != paNoError) goto error;
  printf("I think it worked...\n");
  printf("starting stream\n");
  Pa_StartStream( stream );
  return;

error:
  Pa_Terminate();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
}

void play_sample(struct Module *m, int n)
{
  //data = m->samples[n].sample_data;
  length = m->samples[n].length;
  printf("starting stream\n");
  Pa_StartStream( stream );
  Pa_Sleep( 2 * 1000 );
  Pa_StopStream( stream );
}
