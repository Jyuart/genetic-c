build:
	gcc -Wall main.c -lSDL2 -lm -o result

run:
	./result

clean:
	rm result
