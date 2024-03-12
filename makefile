build:
	gcc memorysystem.c -o ms -Wall

rebuild:
	rm ms
	gcc memorysystem.c -o ms -Wall

clean:
	rm ms