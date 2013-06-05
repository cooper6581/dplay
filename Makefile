test:
	gcc -std=gnu99 -g -Wall -o test loader.c common.c test.c mixer.c player.c -lrt -lasound -lpthread -lportaudio

clean:
	rm -rf *.o test
