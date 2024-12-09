EXEC=audio-vis

CLIB= -I ./lib/include/ -L ./lib/libraries -lportaudio -lglfw3 -lGL -lfftw3f -lm -lpthread -lasound -ljack

${EXEC}: ./src/*
	g++ -o $@ $^ ${CLIB}

clean:
	rm ${EXEC};
.PHONY: clean