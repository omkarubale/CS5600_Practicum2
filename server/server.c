/*
 * server.c -- TCP Socket Server
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

int socket_desc, client_sock;
socklen_t client_size;
struct sockaddr_in server_addr, client_addr;
char server_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE], client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
char client_command[CLIENT_COMMAND_SIZE];

void server_closeServerSocket()
{
  close(client_sock);
  close(socket_desc);
  printf("EXIT: closing server socket\n");
  exit(1);
}

void init_createServerSocket()
{
  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("ERROR: Error while creating socket\n");
    server_closeServerSocket();
  }
  printf("INIT: Socket created successfully\n");
}

void init_bindServerSocket()
{
  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

  // Bind to the set port and IP:
  if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("ERROR: Couldn't bind to the port\n");
    server_closeServerSocket();
  }
  printf("INIT: Done with binding\n");
}

void initServer()
{
  init_createServerSocket();
  init_bindServerSocket();
}

void server_respond(char server_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE])
{
  strcpy(server_message, "This is the server's response message.");

  if (send(client_sock, server_message, strlen(server_message), 0) < 0)
  {
    printf("ERROR: Can't send\n");
    server_closeServerSocket();
  }
}

void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");

  // TODO

  printf("COMMAND: GET complete\n");
}

void command_info(char *remote_file_path)
{
  printf("COMMAND: INFO started\n");

  // TODO

  printf("COMMAND: INFO complete\n");
}

void command_makeDirectory(char *folder_path)
{
  printf("COMMAND: MD started\n");

  // TODO

  printf("COMMAND: MD complete\n");
}

void command_put(char *local_file_path, char *remote_file_path)
{
  printf("COMMAND: PUT started\n");

  // TODO

  printf("COMMAND: PUT complete\n");
}

void command_remove(char *path)
{
  printf("COMMAND: RM started\n");

  // TODO

  printf("COMMAND: RM complete\n");
}

void server_listenForClients()
{
  if (listen(socket_desc, 1) < 0)
  {
    printf("ERROR: Error while listening\n");
    server_closeServerSocket();
  }
  printf("\nListening for incoming connections.....\n");

  // Accept an incoming connection:
  client_size = sizeof(client_addr);
  client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

  if (client_sock < 0)
  {
    printf("ERROR: Can't accept\n");
    server_closeServerSocket();
  }
  printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

void server_listenForCommand()
{
  while (true)
  {
    memset(client_command, 0, sizeof(client_command));
    if (recv(client_sock, client_command, sizeof(client_command), 0) < 0)
    {
      printf("ERROR: Couldn't receive\n");
      server_closeServerSocket();
    }

    printf("Msg from client: %s\n", client_command);

    // Interpret entered command
    char *pch;
    pch = strtok(client_command, " ");

    char *args[3];

    args[0] = pch;
    pch = strtok(NULL, " ");

    // Set arguement Limits based on first argument
    int argcLimit;
    int argc = 1;

    if (strcmp(args[0], "C:001") == 0)
    {
      argcLimit = 3;
    }
    else if (strcmp(args[0], "C:002") == 0)
    {
      argcLimit = 2;
    }
    else if (strcmp(args[0], "C:003") == 0)
    {
      argcLimit = 3;
    }
    else if (strcmp(args[0], "C:004") == 0)
    {
      argcLimit = 2;
    }
    else if (strcmp(args[0], "C:005") == 0)
    {
      argcLimit = 2;
    }
    else if (strcmp(args[0], "C:999") == 0)
    {
      argcLimit = 1;
    }
    else
    {
      printf("ERROR: Invalid command provided\n");
      server_respond("E:404 Invalid command");
      continue;
    }

    // Parse remaining arguements based on set command

    while (pch != NULL)
    {
      if (argc >= argcLimit)
      {
        printf("ERROR: Invalid number of arguements provided\n");
        // continue;
      }

      args[argc++] = pch;
      pch = strtok(NULL, " ");
    }

    // Redirect to correct command
    if (strcmp(args[0], "C:001") == 0)
    {
      command_get(args[1], args[2]);
    }
    else if (strcmp(args[0], "C:002") == 0)
    {
      command_info(args[1]);
    }
    else if (strcmp(args[0], "C:003") == 0)
    {
      command_put(args[1], args[2]);
    }
    else if (strcmp(args[0], "C:004") == 0)
    {
      command_makeDirectory(args[1]);
    }
    else if (strcmp(args[0], "C:005") == 0)
    {
      command_remove(args[1]);
    }
    else if (strcmp(args[0], "C:999") == 0)
    {
      printf("QUIT: Client quiting\n");
      return;
    }
    else
    {
      printf("ERROR: Invalid command provided\n");
    }
  }
}

int main(void)
{
  // Initialize server socket and bind to port:
  initServer();

  // Listen for clients:
  server_listenForClients();

  // Receive client's message:
  server_listenForCommand();

  // // Respond to client:
  // server_respond();

  // Closing server socket:
  server_closeServerSocket();

  return 0;
}