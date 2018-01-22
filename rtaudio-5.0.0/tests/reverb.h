/*
typedef char MY_TYPE;
#define FORMAT RTAUDIO_SINT8
*/

/*
typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16
*/

/*
typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24
typedef signed long MY_TYPE;
#define FORMAT RTAUDIO_SINT32
typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
*/

typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64

struct Data{
	MY_TYPE* tmp;
	int LengthTmp;
	MY_TYPE* RI;
	int LengthRI;

	MY_TYPE* FFT_data_RI_x;
	MY_TYPE* FFT_data_x;
	int FFT_Length_pow2;
	MY_TYPE* FFT_result;
	MY_TYPE* FFT_tempon;

	double LIM_release;
	MY_TYPE LIM_e;
	MY_TYPE LIM_gain;
	double LIM_k;
	double LIM_fa;
	MY_TYPE LIM_liss;
};

typedef struct Data Data;


int Reverb(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
	double /*streamTime*/, RtAudioStreamStatus status, void *data);

void usage(void);

Data* dataInit(Data* data, int length, MY_TYPE* tempon, MY_TYPE* data_RI, MY_TYPE* FFT_data_RI_x, MY_TYPE* FFT_data_x, int FFT_Length_pow2, MY_TYPE* FFT_result, MY_TYPE* FFT_tempon);
