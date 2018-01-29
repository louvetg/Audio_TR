#include "RtAudio.h"
#include "reverb.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <algorithm>
#include <cmath>
#include "somefunc.cpp"


int limiter(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data){

	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	int i;

	MY_TYPE* inBuf = (MY_TYPE*)inputBuffer;
	MY_TYPE* outBuf = (MY_TYPE*)outputBuffer;

	Data* pData = (Data*)data;
		
	double LIM_release = pData->LIM_release; //facteur de release
	MY_TYPE LIM_e = pData->LIM_e; //Enveloppe
	MY_TYPE LIM_gain = pData->LIM_gain; //Gain instantanné 
	double LIM_k = pData->LIM_k;
	double LIM_fa = pData->LIM_fa;//facteur d'attaque
	MY_TYPE LIM_liss = pData->LIM_liss;//Signal lissé de l'instant t - 1
	
	int L_Buff = nBufferFrames;
	
	double t1 = get_process_time();

	for(i = 0; i < L_Buff ; i++){
		LIM_e = std::max((MY_TYPE)std::abs(inBuf[i]),(MY_TYPE)(LIM_e * LIM_release));
		//std::cout << "\n"<< LIM_e;
		if(LIM_e < LIM_k){
			LIM_gain = 1.0;
		}
		else{
			LIM_gain = 1.0 + LIM_k - LIM_e;
		}
		//std::cout << "\n"<< LIM_gain;
		LIM_liss = LIM_liss * LIM_fa + LIM_gain * (1-LIM_fa);
		outBuf[i] = LIM_liss * inBuf[i];
		std::cout << inBuf[i] << ";" << LIM_e << ";" << LIM_gain << ";" << LIM_liss << ";" << outBuf[i] << "\n";

		 
	}
	
	double t2 = get_process_time();
	printf("delta t = %lf\n", t2 - t1);
	
	return 0;
}

int fftReverb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data){

	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	//Déclarations et initialisation
	int n;
	int k;

	int L_Buff = nBufferFrames;

	Data* pData = (Data*)data;
	int L_RI = pData->LengthRI;
	int FFT_Length_pow2 = pData->FFT_Length_pow2;
	MY_TYPE* FFT_data_RI_x = (MY_TYPE*)(pData->FFT_data_RI_x);

	MY_TYPE* FFT_data_x = (MY_TYPE*)(pData->FFT_data_x);
	MY_TYPE* FFT_result_x = (MY_TYPE*)(pData->FFT_result);

	MY_TYPE* FFT_data_y = FFT_data_x + FFT_Length_pow2;
	MY_TYPE* FFT_result_y = FFT_result_x + FFT_Length_pow2;
	MY_TYPE* FFT_data_RI_y = FFT_data_RI_x + FFT_Length_pow2;  

	MY_TYPE* tempon = (MY_TYPE*)(pData->FFT_tempon);

	MY_TYPE* inBuf = (MY_TYPE*)inputBuffer;
	MY_TYPE* outBuf = (MY_TYPE*)outputBuffer;

	
	double t1 = get_process_time();
	
	for(k=0; k < L_Buff; k++){FFT_data_x[k] = inBuf[k];}
	for(k=L_Buff; k < 2*FFT_Length_pow2; k++){ FFT_data_x[k] = 0;}


	fftr(FFT_data_x, FFT_data_y, FFT_Length_pow2);
	
	//(a+ib)(c+id) = ac - bd + i(ad+cd) avec a+ib la RI et c+id la TF de l'entrée
	for (n = 0; n < FFT_Length_pow2; n++){
		FFT_result_x[n] = FFT_data_x[n] * FFT_data_RI_x[n] - FFT_data_y[n] * FFT_data_RI_y[n];
		FFT_result_y[n] = FFT_data_x[n] * FFT_data_RI_y[n] + FFT_data_y[n] * FFT_data_RI_x[n];	
	}

	ifft(FFT_result_x, FFT_result_y, FFT_Length_pow2);
		

	//overlap add
	//Attention cette écriture ne foncrionne que si L_RI est plus longue que L_Buff
	for(n = 0; n < L_Buff + L_RI -1; n++) {
		if (n < L_RI+1){
			if (n < L_Buff){ 
				outBuf[n] = FFT_result_x[n] + tempon[n];	
			}
			else{ 
				tempon[n - L_Buff] = FFT_result_x[n] + tempon[n]; 
			}
		}
		else{ tempon[n - L_Buff] = FFT_result_x[n]; }
	}

	double t2 = get_process_time();
	printf("delta t = %lf\n", t2 - t1);

	return 0;
}

int Reverb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, 
	double /*streamTime*/, RtAudioStreamStatus status, void *data){
	
	
	//Déclarations et initialisation
	int n;
	int k;
	int kmin;
	int kmax;

	MY_TYPE tmp = 0.0;

	int L_Buff = nBufferFrames;

	Data* pData = (Data*)data;
	int L_RI = pData->LengthRI;
	MY_TYPE* repImp = (MY_TYPE*)(pData->RI);
	MY_TYPE* tempon = (MY_TYPE*)(pData->tmp);

	MY_TYPE* inBuf = (MY_TYPE*)inputBuffer;
	MY_TYPE* outBuf = (MY_TYPE*)outputBuffer;

	double t1 = get_process_time();

	if (status) std::cout << "Stream over/underflow detected." << std::endl;

	
	//overlap add
	//Attention cette écriture ne foncrionne que si L_RI est plus longue que L_Buff
	
	for (n = 0; n < (L_Buff + L_RI - 1); n++){

		tmp = 0.0;

		if (n > L_RI){ kmin = n - L_RI + 1; }
		else { kmin = 0; }

		if (n < L_Buff) { kmax = n - 1; }
		else { kmax = L_Buff - 1; }

		for (k = kmin; k <= kmax; k++){
			tmp += inBuf[k] * repImp[n - k + 1];
		}

		if (n < L_RI+1){
			if (n < L_Buff){ 
				outBuf[n] = tmp + tempon[n];
			}
			else{ 
				tempon[n - L_Buff] = tmp + tempon[n]; 
			}
		}
		else{ tempon[n - L_Buff] = tmp; }
	}

	double t2 = get_process_time();

	printf("delta t = %lf\n", t2 - t1);
	//memcpy(outbuff, outBuf, 512 * sizeof(MY_TYPE));
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int channels, fs, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;
	
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
	unsigned int bufferFrames = 256;
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
	

	//Déclaration et initialisation
	int length = 0;
	
	MY_TYPE* data_RI;
	MY_TYPE* tempon;
	Data* data = NULL;
	MY_TYPE* FFT_data_RI_x;
	MY_TYPE* FFT_data_RI_y;
	MY_TYPE* FFT_data_x;
	MY_TYPE* FFT_result;
	MY_TYPE* FFT_tempon;

	FILE * fichier = fopen("./impres", "rb");

		
	if (fichier == NULL) {
		std::cerr << "Error File Opening\n";
		exit(1);
	}


	fseek(fichier, 0, SEEK_END);
	int length_init; 
	length_init = ftell(fichier) / sizeof(MY_TYPE);
	rewind(fichier);

	length = 10000; //20000; 

	printf("longueur du fichier :%d\n", length);

	data_RI = (MY_TYPE *)calloc(length, sizeof(MY_TYPE));
	tempon = (MY_TYPE *)calloc(length - 1, sizeof(MY_TYPE));

	if (data_RI == NULL || tempon == NULL) {
		std::cerr << "Error allocating data_RI or tempon\n";
		exit(1);
	};

	//Reading RI file

	if (fread(data_RI, sizeof(MY_TYPE), length, fichier) != (unsigned)length){
		std::cerr << "Error in fread function\n";
		exit(1);
	}

	int FFT_Length = length + bufferFrames - 1;
	int FFT_Length_pow2 = get_nextpow2(FFT_Length);



	FFT_data_RI_x = (MY_TYPE *)calloc(2 * FFT_Length_pow2, sizeof(MY_TYPE));
	FFT_data_RI_y = FFT_data_RI_x + FFT_Length_pow2;

	FFT_data_x = (MY_TYPE *)calloc(2 * FFT_Length_pow2, sizeof(MY_TYPE));
	FFT_result = (MY_TYPE *)calloc(2 * FFT_Length_pow2, sizeof(MY_TYPE));

	//FFT_tempon probablement beaucoup trop long, pourrait faire seulement la longueur de la RI 
	FFT_tempon = (MY_TYPE *)calloc(2 * FFT_Length_pow2 - bufferFrames, sizeof(MY_TYPE));

	if (FFT_data_RI_x == NULL || FFT_data_x == NULL || FFT_result == NULL) {
		std::cerr << "Error allocating FFT_data_RI_x or FFT_data_x or FFT_result\n";
		exit(1);
	};

	
	for(int i = 0; i < length ; i++){FFT_data_RI_x[i]= data_RI[i];} 
	
	fftr(FFT_data_RI_x, FFT_data_RI_y, FFT_Length_pow2);


	data = (Data*)malloc(sizeof(Data*));
	data = dataInit(data, length, tempon, data_RI, FFT_data_RI_x, FFT_data_x, FFT_Length_pow2, FFT_result, FFT_tempon);

	//Les fonctions Reverb, fftReverb et limiter sont disponibles
	try {
		adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &Reverb, data, &options);
		//adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &fftReverb, data, &options);
		//adac.openStream(&oParams, &iParams, FORMAT, fs, &bufferFrames, &limiter, data, &options);
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

Data* dataInit(		   Data* data, 
			   int length, 
			   MY_TYPE* tempon, 
			   MY_TYPE* data_RI, 
			   MY_TYPE* FFT_data_RI_x, 
			   MY_TYPE* FFT_data_x,
			   int FFT_Length_pow2,
			   MY_TYPE* FFT_result,
			   MY_TYPE* FFT_tempon
			   ){
	
	data->LengthRI = length;
	data->LengthTmp = length - 1;
	data->RI = data_RI;
	data->tmp = tempon;

	data->FFT_data_RI_x = FFT_data_RI_x;
	data->FFT_data_x = FFT_data_x;
	data->FFT_Length_pow2 = FFT_Length_pow2;
	data->FFT_result = FFT_result;
	data->FFT_tempon = FFT_tempon;

	data->LIM_release = 0.8;
	data->LIM_e = 0;
	data->LIM_gain = 0;
	data->LIM_k = 0.01;
	data->LIM_fa = 0.01;
	data->LIM_liss = 0;

	return data;
}
