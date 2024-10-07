#include <filterHandler.h>
#include <cmath>
#include <main.h>

int fftSize;
int fftSampleRate;
fftwComplex* fftIn;
fftwComplex* fftOut;
fftwPlan plan;
fftwPlan inverse_plan;

void filter_init(int bufferSize, int sampleRate)
{
    fftSize = bufferSize;
    fftSampleRate = sampleRate;
    fftIn = reinterpret_cast<fftwComplex*>(fftwf_malloc(sizeof(fftwComplex) * fftSize));
    fftOut = reinterpret_cast<fftwComplex*>(fftwf_malloc(sizeof(fftwComplex) * fftSize));

    // For testing purposes
    plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
    inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_ESTIMATE);
    // More Quality (Read Wisdom)
    // if (fftwf_import_wisdom_from_filename("wisdom"))
    // {
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_EXHAUSTIVE | FFTW_WISDOM_ONLY);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_EXHAUSTIVE | FFTW_WISDOM_ONLY);
    // } else
    // {
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_EXHAUSTIVE);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_EXHAUSTIVE);
    //     fftwf_export_wisdom_to_filename("wisdom");
    // }
}
bool fftw_filter(fftwType* source, fftwType* array, fftwType minFreq, fftwType maxFreq)
{
    int min = minFreq*fftSize/fftSampleRate;
    int max = maxFreq*fftSize/fftSampleRate;
    for (size_t i = 0; i < fftSize; i++)
    {
        fftIn[i][0] = source[i];
        fftIn[i][1] = 0;
    }
    // Run the fft filter
    fftwf_execute(plan);
    
    // Calculate fft using single pass
    for (size_t i = 0; i < fftSize-max; i++)
    {
        int index = i;
        if (i>min) index = max+i;
        fftOut[index][0] = 0;
        fftOut[index][1] = 0;
    }
    fftwf_execute(inverse_plan);
    for (size_t i = 0; i < fftSize; i++)
    {
        array[i] = fftIn[i][0]/fftSize;
    }

    return 1;
}

void filter_end()
{
    fftwf_destroy_plan(plan);
    fftwf_destroy_plan(inverse_plan);
    fftwf_free(fftOut);
}