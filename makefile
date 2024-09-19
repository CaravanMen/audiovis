default:
	g++ -o audio_vis ./src/*.cpp ./src/*.c -I ./lib/include/ -lglfw -lGL -lpulse-simple -lpulse -lfftw3f -lm 
	__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./audio_vis;
build:
	g++ -o audio_vis ./src/*.cpp ./src/*.c -I ./lib/include/ -lglfw -lGL -lpulse-simple -lpulse -lfftw3f -lm
clean:
	rm ./audio_vis;
run:
	./audio_vis;