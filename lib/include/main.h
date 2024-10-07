#define PA_DEV_NAME ".monitor"
#define SAMPDTL 1024
#define MINFREQ 30
#define MAXFREQ 1200
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define WIDTH 512
#define HEIGHT 512
// Simply set the number of rows to the ammount of the "selected" buffer space (buffer space that MINFREQ and MAXFREQ occupy) (MINFREQ < i < MAXFREQ)
constexpr int BUFSIZE = (SAMPLE_RATE)/(MAXFREQ-MINFREQ)*SAMPDTL;