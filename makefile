all: ./server/server ./client/fget ./client/testing
	echo "MAKE: Building all"

client/testing: ./client/testing.c ./common/common.h ./server/server ./client/fget 
	echo "MAKE: Building Testing"
	gcc -Wall ./client/testing.c -o ./client/testing

server/server: ./server/server.c ./common/common.h ./server/configserver.h
	echo "MAKE: Building Server"
	gcc -Wall ./server/server.c -o ./server/server

client/fget: ./client/client.c ./common/common.h
	echo "MAKE: Building Client"
	gcc -Wall ./client/client.c -o ./client/fget

clean:
	rm -f ./server/server ./client/fget ./client/testing