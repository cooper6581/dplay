#include "loader.h"

static char * load_file(char *fname);

int load_module(char *fname)
{
  char *buffer = NULL;
  char sig[5];

  buffer = load_file(fname);
  if (buffer == NULL) {
    fprintf(stderr,"Problem loading file, exiting\n");
    if (buffer != NULL)
      free(buffer);
    return 1;
  }
  // check if it's a valid MOD
  if (buffer == NULL)
    printf("BUFFER IS NULL!\n");
  memcpy(sig,&buffer[1080],4);
  sig[4] = '\0';
  if (strncmp(sig,"M.K.",4) == 0)
    fprintf(stderr,"Yay, valid 4 chan mod!\n");
  else if (strncmp(sig,"6CHN",4) == 0)
    fprintf(stderr,"Yay, valid 6 chan mod!\n");
  else if (strncmp(sig,"8CHN",4) == 0)
    fprintf(stderr,"Yay, valid 8 chan mod!\n");
  else {
    fprintf(stderr,"Not a valid mod file, missing signature\n");
    free(buffer);
    return 2;
  }

  return 0;
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
