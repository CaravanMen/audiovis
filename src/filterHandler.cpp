#include <filterHandler.h>
#include <main.h>

// Include libraries
#include <cmath>
#include <memory.h>
#include <string>

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

    // // Generate wisdom for more quality
    // if (fftwf_import_wisdom_from_filename("wisdom"))
    // {
    //     // If wisdom already exists, use it
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_EXHAUSTIVE | FFTW_WISDOM_ONLY);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_EXHAUSTIVE | FFTW_WISDOM_ONLY);
    // } else
    // {
    //     plan = fftwf_plan_dft_1d(fftSize, fftIn, fftOut, FFTW_FORWARD, FFTW_EXHAUSTIVE);
    //     inverse_plan = fftwf_plan_dft_1d(fftSize, fftOut, fftIn, FFTW_BACKWARD, FFTW_EXHAUSTIVE);
    //     fftwf_export_wisdom_to_filename("wisdom");
    // }
}

bool fftw_filter(fftwType* arrayIn, fftwType* arrayOut, size_t size, fftwType minFreq, fftwType maxFreq, fftwType* ampOut)
{
    int min = static_cast<int>(floor((minFreq*fftSize)/fftSampleRate));
    int max = static_cast<int>(ceil((maxFreq*fftSize)/fftSampleRate));
    // printf("min: %i, max: %i\n", min, max);
    
    for (size_t i=0; i < fftSize; i++)
    {
        if (i < size)
        {
            fftIn[i][0] = arrayIn[i];
        } else {
            fftIn[i][0] = 0;
        }
        fftIn[i][1] = 0;
    }
    
    // Run the fft filter
    fftwf_execute(plan);
    // Output filter and find loudest part
    fftwType amp = 0.0f; // Amplitude
    for (size_t i = 0; i < max-min; i++)
    {
        int index = min+i;
        fftwType real = fftOut[index][0];
        fftwType imag = fftOut[index][1];
        fftwType sect = sqrt((real*real)+(imag*imag));
        if (arrayOut != nullptr)
        {
            arrayOut[i] = sect;
        }
        // Check amp
        if (amp < sect)
        {
            amp = sect;
        }
    }

    if (ampOut != nullptr)
    {
        *ampOut = amp;
    }
    return 1;
}

void filter_end()
{
    fftwf_destroy_plan(plan);
    fftwf_free(fftOut);
}