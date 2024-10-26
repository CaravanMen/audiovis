default:
	g++ -o audio_vis ./src/*.cpp ./src/*.c -I ./lib/include/ -L ./lib/libraries -lrtaudio -lglfw3 -lGL -lpulse-simple -lpulse -lfftw3f -lm
	./audio_vis
	rm ./audio_vis
build:
	g++ -o audio_vis ./src/*.cpp ./src/*.c -I ./lib/include/ -L ./lib/libraries -lrtaudio -lglfw3 -lGL -lpulse-simple -lpulse -lfftw3f -lm
clean:
	rm ./audio_vis;
run:
	./audio_vis;