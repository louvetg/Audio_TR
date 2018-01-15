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



void usage(void) {
	// Error function in case of incorrect command-line
	// argument specifications
	std::cout << "\nuseage: duplex N fs <iDevice> <oDevice> <iChannelOffset> <oChannelOffset>\n";
	std::cout << "    where N = number of channels,\n";
	std::cout << "    fs = the sample rate,\n";
	std::cout << "    iDevice = optional input device to use (default = 0),\n";
	std::cout << "    oDevice = optional output device to use (default = 0),\n";
	std::cout << "    iChannelOffset = an optional input channel offset (default = 0),\n";
	std::cout << "    and oChannelOffset = optional output channel offset (default = 0).\n\n";
	exit(0);
}

int inout(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	unsigned int *bytes = (unsigned int *) ((pData) data) -> bufferBytes ;
	memcpy(outputBuffer, inputBuffer, *bytes);
	return 0;
}

int Reverb(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	unsigned int *bytes = (unsigned int *)((pData)data)->bufferBytes;
	//memcpy(outputBuffer, inputBuffer, *bytes);

	/*	//initialisation de data
	data = (pData)malloc(sizeof(pData));
	data->bufferBytes = &bufferBytes;
	data->LengthRI = length;
	data->LengthTmp = length - 1;
	data->RI = (MY_TYPE*)malloc(length*sizeof(MY_TYPE));
	data->tmp = temp;*/
	
	int n;
	int L_OA = *bytes;
	int M = ((pData)data)->LengthRI;
	MY_TYPE tmpt = 0;
	int kmin;
	int kmax;
	int k;
	MY_TYPE* imp_left = ((pData)data)->RI;
	MY_TYPE* bufferIn = (MY_TYPE*)inputBuffer;
	MY_TYPE* bufferOut = (MY_TYPE*)outputBuffer;
	MY_TYPE* temp = ((pData)data)->tmp;



	for (n = 1; n <= (L_OA + M - 1); n++){		tmpt = 0;				if (n >= M){ kmin = n - M + 1;}		else {kmin = 1;}				if (n < L_OA) {kmax = n; }		else { kmax = L_OA; }				for (k = kmin; k <= kmax; k++){
			tmpt = tmpt + bufferIn[k]*imp_left[n - k + 1];
		}
		
		if (n < L_OA){bufferOut[n] = tmpt + temp[n];  }
		else{temp[n - L_OA] = tmpt;}	}	

	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int channels, fs, bufferBytes, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;
	FILE * fichier = NULL;
	MY_TYPE* data_RI; //bug surement ici
	MY_TYPE* temp;
	//Ouverture du ficher rep imp à modifier
	fichier = fopen("./ressources_tstr_v1_1/c/impres", "rb");
	if (fichier == NULL) {
		std::cerr << "Error File Opening\n";
		exit(1);
	}


	fseek(fichier, 0, SEEK_END);
	int length = ftell(fichier);
	rewind(fichier);

	printf("longueur du fichier :%d\n", length);

	data_RI = (MY_TYPE *)calloc(length, sizeof(MY_TYPE));
	temp = (MY_TYPE *)calloc(length - 1, sizeof(MY_TYPE));


	// printf("taille allouée: %d soit %d mots de MY_TYPE\n", sizeof(data_RI[length - 1]) , sizeof(data_RI)/sizeof(MY_TYPE));

	if (data_RI == NULL) {
		std::cerr << "Error allocating data_RI\n";
		exit(1);
	};


	// Minimal command-line checking
	if (argc < 3 || argc > 7) usage();

	RtAudio adac;
	if (adac.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(1);
	}

	channels = (unsigned int)atoi(argv[1]);
	fs = (unsigned int)atoi(argv[2]);
	if (argc > 3)
		iDevice = (unsigned int)atoi(argv[3]);
	if (argc > 4)
		oDevice = (unsigned int)atoi(argv[4]);
	if (argc > 5)
		iOffset = (unsigned int)atoi(argv[5]);
	if (argc > 6)
		oOffset = (unsigned int)atoi(argv[6]);

	// Let RtAudio print messages to stderr.
	adac.showWarnings(true);

	// Set the same number of channels for both input and output.
	unsigned int bufferFrames = 512;
	RtAudio::StreamParameters iParams, oParams;
	iParams.deviceId = iDevice;
	iParams.nChannels = channels;
	iParams.firstChannel = iOffset;
	oParams.deviceId = oDevice;
	oParams.nChannels = channels;
	oParams.firstChannel = oOffset;

	if (iDevice == 0)
		iParams.deviceId = adac.getDefaultInputDevice();
	if (oDevice == 0)
		oParams.deviceId = adac.getDefaultOutputDevice();

	RtAudio::StreamOptions options;
	//options.flags |= RTAUDIO_NONINTERLEAVED;

	

	bufferBytes = bufferFrames * channels * sizeof(MY_TYPE);




	//Reading RI file

	int N_Data = fread(data_RI, 1, length, fichier);
	if (N_Data != length){
		std::cerr << "Error in fread function\n";
		exit(1);
	}

	int i = 0;
	for (i = 0; i < length; i++){
		if (data_RI[i] != 0){
		//printf("element num: %d egal %d \n",i,data_RI[i]);
		}
	}


	pData data = NULL;

	//initialisation de data
	data = (pData)malloc(sizeof(pData));
	data->bufferBytes = &bufferBytes;
	data->LengthRI = length - 70000;
	data->LengthTmp = length - 1 - 70000;
	data->RI = (MY_TYPE*)malloc(length*sizeof(MY_TYPE));
	data->tmp = temp;
	


	try {
		adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &Reverb, data, &options);
	}
	catch (RtAudioError& e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		exit(1);
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
	catch (RtAudioError& e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		goto cleanup;
	}

cleanup:
	if (adac.isStreamOpen()) adac.closeStream();

	return 0;
}