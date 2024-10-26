EXEC=audio-vis

${EXEC}: 
	g++ -o ${EXEC} ./src/*.cpp ./src/*.c -I./lib/include/ -L./lib/libraries -lglfw3 -lGL -lpulse-simple -lpulse -lfftw3f -lm 
	./${EXEC}
build:
	g++ -o ${EXEC} ./src/*.cpp ./src/*.c -I./lib/include/ -L./lib/libraries -lglfw3 -lGL -lpulse-simple -lpulse -lfftw3f -lm
.PHONY: build
clean:
	rm ./${EXEC};
.PHONY: clean