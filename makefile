EXEC=audio-vis

CLIB=-I./lib/include/ -L./lib/libraries -lglfw3 -lGL -lpulse-simple -lpulse -lfftw3f -lm

${EXEC}: ./src/*.cpp ./src/*.c
	g++ -o $@ $^ ${CLIB};
clean:
	rm ./${EXEC};
.PHONY: clean