/*
 * client.c -- TCP Socket Client
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../common/common.h"

int socket_desc;
struct sockaddr_in server_addr;
char server_message[ERROR_CODE_SIZE + ERROR_CODE_PADDING + SERVER_MESSAGE_SIZE], client_message[ERROR_CODE_SIZE + ERROR_CODE_PADDING + CLIENT_MESSAGE_SIZE];

void client_closeClientSocket()
{
  // Close the socket:
  close(socket_desc);
  printf("EXIT: closing client socket\n");
  exit(1);
}

void init_createClientSocket()
{
  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("ERROR: Unable to create socket\n");
    client_closeClientSocket();
  }

  printf("INIT: Socket created successfully\n");
}

void init_initClient()
{
  init_createClientSocket();
}

void client_connect()
{
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  // Send connection request to server:
  if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("ERROR: Unable to connect\n");
    client_closeClientSocket();
  }
  printf("Connected with server successfully\n");
}

void client_getMessage()
{
  // Get input from the user:
  printf("Enter message: ");
  gets(client_message);
}

void client_sendMessageToServer()
{
  // Send the message to server:
  if (send(socket_desc, client_message, strlen(client_message), 0) < 0)
  {
    printf("ERROR: Unable to send message\n");
    client_closeClientSocket();
  }
}

void client_recieveResponse()
{
  // Receive the server's response:
  if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
  {
    printf("ERROR: Error while receiving server's msg\n");
    client_closeClientSocket();
  }

  printf("Server's response: %s\n", server_message);
}

void command_get(char *remote_file_path, char *local_file_path)
{
}

void command_info(char *remote_file_path)
{
}

void command_makeDirectory(char *folder_path)
{
}

void command_put(char *local_file_path, char *remote_file_path)
{
}

void command_remove(char *path)
{
}

int main(void)
{
  // Initialize client socket:
  init_initClient();

  // Connect to server socket:
  client_connect();

  // Get text message to send to server:
  client_getMessage();
  // Send text message to server:
  client_sendMessageToServer();
  // Recieve response from server:
  client_recieveResponse();

  // Closing client socket:
  client_closeClientSocket();

  return 0;
}