CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c parson.c json_helpers.c
	$(CC) -o client client.c requests.c helpers.c buffer.c parson.c json_helpers.c -Wall

run: client
	./client

clean:
	rm -f *.o client
