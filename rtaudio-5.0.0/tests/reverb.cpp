/******************************************/
/*
  duplex.cpp
  by Gary P. Scavone, 2006-2007.

  This program opens a duplex stream and passes
  input directly through to the output.
*/
/******************************************/

#include "RtAudio.h"
#include "reverb.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <errno.h>



void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications
  std::cout << "\nuseage: duplex N fs <iDevice> <oDevice> <iChannelOffset> <oChannelOffset>\n";
  std::cout << "    where N = number of channels,\n";
  std::cout << "    fs = the sample rate,\n";
  std::cout << "    iDevice = optional input device to use (default = 0),\n";
  std::cout << "    oDevice = optional output device to use (default = 0),\n";
  std::cout << "    iChannelOffset = an optional input channel offset (default = 0),\n";
  std::cout << "    and oChannelOffset = optional output channel offset (default = 0).\n\n";
  exit( 0 );
}

int inout( void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
           double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
  // Since the number of input and output channels is equal, we can do
  // a simple buffer copy operation here.
  if ( status ) std::cout << "Stream over/underflow detected." << std::endl;

  unsigned int *bytes = (unsigned int *) data;
  memcpy( outputBuffer, inputBuffer, *bytes );
  return 0;
}

int main( int argc, char *argv[] )
{
  unsigned int channels, fs, bufferBytes, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;
  FILE * fichier = NULL;
  MY_TYPE* data_RI; //bug surement ici

  //Ouverture du ficher rep imp à modifier
  fichier =  fopen("/users/phelma/phelma2015/louvetg/TR_audio/ressources_tstr_v1_1/c/impres","rb");	
  if(fichier == NULL) {
     std::cerr << "Error File Opening\n";
     exit( 1 );
  }
  

  fseek(fichier,0,SEEK_END);
  int length = ftell(fichier);  
  rewind(fichier);
  
  printf("longueur du fichier :%d\n", length);

  data_RI = (MY_TYPE *)calloc(length,sizeof(MY_TYPE));
  
  // printf("taille allouée: %d soit %d mots de MY_TYPE\n", sizeof(data_RI[length - 1]) , sizeof(data_RI)/sizeof(MY_TYPE));

  if( data_RI == NULL ) { 
    std::cerr << "Error allocating data_RI\n";
    exit( 1 );
  };


  // Minimal command-line checking
  if (argc < 3 || argc > 7 ) usage();

  RtAudio adac;
  if ( adac.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 1 );
  }

  channels = (unsigned int) atoi(argv[1]);
  fs = (unsigned int) atoi(argv[2]);
  if ( argc > 3 )
    iDevice = (unsigned int) atoi(argv[3]);
  if ( argc > 4 )
    oDevice = (unsigned int) atoi(argv[4]);
  if ( argc > 5 )
    iOffset = (unsigned int) atoi(argv[5]);
  if ( argc > 6 )
    oOffset = (unsigned int) atoi(argv[6]);

  // Let RtAudio print messages to stderr.
  adac.showWarnings( true );

  // Set the same number of channels for both input and output.
  unsigned int bufferFrames = 512;
  RtAudio::StreamParameters iParams, oParams;
  iParams.deviceId = iDevice;
  iParams.nChannels = channels;
  iParams.firstChannel = iOffset;
  oParams.deviceId = oDevice;
  oParams.nChannels = channels;
  oParams.firstChannel = oOffset;

  if ( iDevice == 0 )
    iParams.deviceId = adac.getDefaultInputDevice();
  if ( oDevice == 0 )
    oParams.deviceId = adac.getDefaultOutputDevice();

  RtAudio::StreamOptions options;
  //options.flags |= RTAUDIO_NONINTERLEAVED;
  
  pData data = malloc(sizeof(data));
  data = NULL;
    
  bufferBytes = bufferFrames * channels * sizeof( MY_TYPE );
  
  data -> bufferBytes = &bufferBytes;
  //Reading RI file

  int N_Data = fread(data_RI, 1, length, fichier);
  if(N_Data != length ){
    std::cerr << "Error in fread function\n";
    exit( 1 );
  }
  
  int i = 0 ;
  for(i = 0; i < length; i++){
    if(data_RI[i] != 0){/*printf("element num: %d egal %d \n",i,data_RI[i]);*/}
  }
 


  try {
    adac.openStream( &oParams, &iParams, FORMAT, fs, &bufferFrames, &inout, &bufferBytes, &options );
  }
  catch ( RtAudioError& e ) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    exit( 1 );
  }

  // Test RtAudio functionality for reporting latency.
  std::cout << "\nStream latency = " << adac.getStreamLatency() << " frames" << std::endl;

  try {
    adac.startStream();

    char input;
    std::cout << "\nRunning ... press <enter> to quit (buffer frames = " << bufferFrames << ").\n";
    std::cin.get(input);

    // Stop the stream.
    adac.stopStream();
  }
  catch ( RtAudioError& e ) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    goto cleanup;
  }

 cleanup:
  if ( adac.isStreamOpen() ) adac.closeStream();

  return 0;
}
