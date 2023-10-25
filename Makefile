build:
	gcc -Wall main.c -lSDL2 -lSDL2_ttf -lm -lsodium -o result

run:
	./result

clean:
	rm result
