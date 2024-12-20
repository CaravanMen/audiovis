// All required libraries and inclusions
#include <cstddef>
#define PA_DEV_NAME ".monitor"
#define SAMPDTL 1024
#define MINFREQ 30
#define MAXFREQ 1200
#define SAMPLERATE 48000
#define CHANNELS 1
#define WIDTH 512
#define HEIGHT 512
// Simply set the number of rows to the ammount of the "selected" buffer space (buffer space that MINFREQ and MAXFREQ occupy) (MINFREQ < i < MAXFREQ)
constexpr int BUFFSIZE = (SAMPLERATE)/(MAXFREQ-MINFREQ)*SAMPDTL;

// Structures
template <typename T>
struct RadialCircleDataStruct
{
    int maxRings = 16;
    int nextAvailable = 1;
    T array[16+(16*SAMPDTL)];
};