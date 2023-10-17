build:
	gcc -Wall main.c -lSDL2 -o result

run:
	./result

clean:
	rm result
