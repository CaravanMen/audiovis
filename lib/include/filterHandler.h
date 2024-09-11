#include <fftw3.h>
// The format for values that the filter library will take/return
typedef float fftwType;
// The format the fftIn and fftOut variables created using this type
typedef fftwf_complex fftwComplex;
// The format for the fftw plan variables created using this plan type
typedef fftwf_plan fftwPlan;

void filter_init(int sampeSize, int sampleRate);
bool fftw_filter(fftwType* arrayIn, fftwType* arrayOut, fftwType minFreq, fftwType maxFreq);
void filter_end();