test:
	gcc -std=c99 -g -Wall -lrt -lasound -lpthread -lportaudio -o test loader.c test.c mixer.c

clean:
	rm -rf *.o test
