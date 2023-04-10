all: server client

server: ./server/server.c
	gcc -Wall ./server/server.c -o ./server/server

client: ./client/client.c
	gcc -Wall ./client/client.c -o ./client/client
