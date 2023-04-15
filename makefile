all: ./server/server ./client/client
	echo "MAKE: Building all"

server: ./server/server.c
	echo "MAKE: Building Server"
	gcc -W ./server/server.c -o ./server/server

client: ./client/client.c
	echo "MAKE: Building Client"
	gcc -W ./client/client.c -o ./client/client

clean:
	rm -f ./server/server ./client/client