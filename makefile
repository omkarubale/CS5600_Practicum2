all: ./server/server ./client/fget
	echo "MAKE: Building all"

server: ./server/server.c
	echo "MAKE: Building Server"
	gcc -Wall ./server/server.c -o ./server/server

client/fget: ./client/client.c
	echo "MAKE: Building Client"
	gcc -Wall ./client/client.c -o ./client/fget

clean:
	rm -f ./server/server ./client/fget