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
    // More Quality
    // // Read wisdom
    // if (fftwf_import_wisdom_from_filename("wisdom"))
    // {
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE_PATIENT | FFTW_WISDOM_ONLY);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_ESTIMATE_PATIENT | FFTW_WISDOM_ONLY);
    // } else
    // {
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_EXHAUSTIVE);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_EXHAUSTIVE);
    //     fftwf_export_wisdom_to_filename("wisdom");
    // }
}
bool fftw_filter(fftwType* source, fftwType* array, int rows, fftwType minFreq, fftwType maxFreq, fftwType &amp)
{
    int min = floor(minFreq*fftSize/fftSampleRate);
    int max = floor(maxFreq*fftSize/fftSampleRate);
    for (size_t i = 0; i < fftSize; i++)
    {
        fftIn[i][1] = 0;
        if (i<max)
        {
            fftIn[i][0] = source[i];  
        }else fftIn[i][0] = 0;
    }
    // Run the fft filter
    fftwf_execute(plan);
    // Output filter and find loudest part
    fftwType loudest = 0;
    // Calculate fft using single pass
    fftwType fftPass[fftSize];
    for (size_t i = 0; i <= max-min; i++)
    {
        int index = min+i;
        fftwType real = fftOut[index][0];
        fftwType imag = fftOut[index][1];
        fftwType sect = sqrt((real*real)+(imag*imag));
        if (sect > loudest)
        {
            loudest = sect;
        }
        fftPass[i] = sect;
    }
    // Set amp value
    amp = loudest;
    // Store FFT in array
    for (size_t i = 0; i <= max-min; i++)
    {
        array[i]=fftPass[i];
    }

    return 1;
}

void filter_end()
{
    fftwf_destroy_plan(plan);
    fftwf_destroy_plan(inverse_plan);
    fftwf_free(fftOut);
}