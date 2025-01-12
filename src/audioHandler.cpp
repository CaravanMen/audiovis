#include <audioHandler.h>
#include <memory>
#include <math.h>
using namespace std;

PaUtilRingBuffer rBuffer;
float *rBufferData;
int framesPerBuffer;
PaStream *stream;


// Stream Stuff
int audioCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    if (input == nullptr) {
        printf("No input data received.\n");
        return paContinue;
    }
    (void)output;
    const float *in = (const float*) input;
    unsigned long framesWritten = PaUtil_WriteRingBuffer(&rBuffer, in, framesPerBuffer);
    // Check for buffer overflow
    if (framesWritten < framesPerBuffer) {
        printf("Ring buffer overflow! Lost %i frames\n", (framesPerBuffer - framesWritten));
    }
    return paContinue;
}

void InitialiseAudioHandler(int FramesPerBuffer, int RingBufferSize){
    framesPerBuffer = FramesPerBuffer;
    // Variable used to record errors for portaudio is called err:
    PaError err = paNoError;
    // Initialising portaudio
    err = Pa_Initialize();
    checkErr(err);
    // Checking if devices are available:
    int numDevices = Pa_GetDeviceCount();
    printf("Number of devices: %d", numDevices);

    if (numDevices < 0)
    {
        printf("Error getting device count\n");
        exit(EXIT_FAILURE);
    }else if (numDevices == 0)
    {
        printf("There are no available audio devices on this machine\n");
        exit(EXIT_SUCCESS);
    }
    const PaDeviceInfo *deviceInfo;
    for (int i=0; i<numDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("Device %d:\n", i);
        printf("    name: %s\n", deviceInfo->name);
        printf("    maxInputChannels: %d\n", deviceInfo->maxInputChannels);
        printf("    maxOutputChannels: %d\n", deviceInfo->maxOutputChannels);
        printf("    defaultSampleRate: %f\n", deviceInfo->defaultSampleRate);
    }
    rBufferData = (float*)malloc(sizeof(float)*RingBufferSize);
    // Setting up ring buffer for recording and reading stream simultaneously
    if (PaUtil_InitializeRingBuffer(&rBuffer, sizeof(float), RingBufferSize, rBufferData) < 0)
    {
        printf("Failed to initialize ring buffer\n");
        exit(EXIT_FAILURE);
    }
}

void OpenStream(int device, int inChannelCount, int outChannelCount, int sampleRate)
{
    // PaStreamParameters inputParameters;
    // inputParameters.device = device;
    // inputParameters.channelCount = inChannelCount;
    // inputParameters.sampleFormat = paFloat32;
    // inputParameters.hostApiSpecificStreamInfo = NULL;
    // inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
    // PaStreamParameters outputParameters;
    // outputParameters.device = device;
    // outputParameters.channelCount = outChannelCount;
    // outputParameters.sampleFormat = paFloat32;
    // outputParameters.hostApiSpecificStreamInfo = NULL;
    // outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
    PaError err = paNoError;
    // Opening the stream
    err = Pa_OpenDefaultStream(&stream, inChannelCount, outChannelCount, paFloat32, sampleRate, framesPerBuffer, audioCallback, NULL);
    checkErr(err);
}

void CloseStream()
{
    PaError err = paNoError;
    Pa_CloseStream(&stream);
    checkErr(err);
}

void StartStream()
{
    PaError err = paNoError;
    err = Pa_StartStream(stream);
    checkErr(err);
}

void StopStream()
{
    PaError err = paNoError;
    err = Pa_StopStream(stream);
    checkErr(err);
}

void ReadRingBuffer(float *data, int elementCount)
{
    // Read samples from the ring buffer
    float readBuffer[elementCount];
    unsigned long framesAvailable = PaUtil_GetRingBufferReadAvailable(&rBuffer);

    if (framesAvailable > 0) {
        unsigned long framesToRead = min((unsigned long)elementCount, framesAvailable);
        unsigned long framesRead = PaUtil_ReadRingBuffer(&rBuffer, readBuffer, framesToRead);

        *data = *readBuffer;
    }
}

void TerminateAudioHandler()
{
    StopStream();
    CloseStream();
    delete[] rBufferData;
    Pa_Terminate();
}