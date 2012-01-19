test:
	gcc -std=gnu99 -g -Wall -lrt -lasound -lpthread -lportaudio -o test loader.c common.c test.c mixer.c

clean:
	rm -rf *.o test
