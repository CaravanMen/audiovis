
#include <cstdlib>
#include <cstdio>
// PortAudio Stuff
#include <portaudio/portaudio.h>
#include <portaudio/common/pa_ringbuffer.h>


// Error Handling Stuf
inline void checkErr(PaError err)
{
    if (err != paNoError)
    {
        printf("[ERR] Portaudio Error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
}
// Initialization and termination
void InitialiseAudioHandler(int FramesPerBuffer, int RingBufferSize);
void TerminateAudioHandler();

// Stream Stuff
void OpenStream(int device, int inChannelCount, int outChannelCount, int sampleRate);
void CloseStream();
void StartStream();
void StopStream();

// RingBuffer stuff
void ReadRingBuffer(float *data, int elementCount);