all: tetris
	./tetris

tetris: main.c 
	gcc main.c -o tetris -lSDL2
