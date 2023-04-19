all: ./server/server ./client/fget
	echo "MAKE: Building all"

server/server: ./server/server.c ./common/common.h ./server/configserver.h
	echo "MAKE: Building Server"
	gcc -Wall ./server/server.c -o ./server/server

client/fget: ./client/client.c ./common/common.h
	echo "MAKE: Building Client"
	gcc -Wall ./client/client.c -o ./client/fget

clean:
	rm -f ./server/server ./client/fget