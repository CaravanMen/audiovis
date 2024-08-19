// Includes
#include <stdio.h>
// Defined variables
#define SAMPDTL 1024
#define MINFREQ 30
#define MAXFREQ 16000
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define WIDTH 512
#define HEIGHT 512
// Simply set the number of rows to the ammount of the "selected" buffer space (buffer space that MINFREQ and MAXFREQ occupy) (MINFREQ < i < MAXFREQ)
constexpr int BUFSIZE = (SAMPLE_RATE)/(float)(MAXFREQ-MINFREQ)*SAMPDTL;
// The max value of a certain ammout of bytes as a binary string of all 1
constexpr int maxVal()
{
    long int finalNum = 1;
    // how many bytes to include in the count
    for (int i=0; i<32; i++)
    {
        finalNum *= 2;
    }
    return finalNum/2;
}