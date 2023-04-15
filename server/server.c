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
#include <sys/stat.h>
#include "../common/common.h"

#define ROOT_DIRECTORY "./root"

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

#pragma region Init

int init_createServerSocket()
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
    return -1;
  }
  printf("INIT: Socket created successfully\n");
  return 0;
}

int init_bindServerSocket()
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
    return -1;
  }
  printf("INIT: Done with binding\n");
  return 0;
}

int init_createRootDirectory()
{
  struct stat st = {0};

  if (stat(ROOT_DIRECTORY, &st) == -1)
  {
    // created using S_IREAD, S_IWRITE, S_IEXEC (0400 | 0200 | 0100) permission flags
    int res = mkdir(ROOT_DIRECTORY, 0700);
    if (res != 0)
    {
      printf("ERROR: root directory creation failed\n");
      return -1;
    }
  }

  return 0;
}

int initServer()
{
  int status;
  status = init_createServerSocket();
  if (status != 0)
    return -1;
  status = init_bindServerSocket();
  if (status != 0)
    return -1;
  status = init_createRootDirectory();
  if (status != 0)
    return -1;

  return 0;
}

#pragma endregion Init

#pragma region Communication

void server_respond(char *server_message)
{
  printf("RESPONSE: %s\n", server_message);
  if (send(client_sock, server_message, strlen(server_message), 0) < 0)
  {
    printf("ERROR: Can't send\n");
    server_closeServerSocket();
  }
}

#pragma endregion Communication

#pragma region Helpers

int isDirectoryExists(const char *path)
{
  struct stat stats;

  stat(path, &stats);

  // Check for file existence
  if (S_ISDIR(stats.st_mode))
    return 1;

  return 0;
}

#pragma endregion Helpers

#pragma region Commands

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

  char *actual_path;
  strcpy(actual_path, ROOT_DIRECTORY);
  strcat(actual_path, "/");
  strncat(actual_path, folder_path, strlen(folder_path) - 1);

  printf("MD: actual path: %s\n", actual_path);

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  if (isDirectoryExists(actual_path))
  {
    // directory already exists
    printf("MD: Directory already exists\n");

    strcat(response_message, "E:406 ");
    strcat(response_message, "Directory already exists");

    server_respond(response_message);
  }
  else
  {
    // directory doesn't exist
    printf("MD: Directory doesn't exist, creating directory\n");

    int res = mkdir(actual_path, 0700);
    if (res != 0)
    {
      // creation of directory failed
      printf("MD ERROR: directory creation failed\n");
      perror("mkdir");
      strcat(response_message, "E:406 ");
      strcat(response_message, "Directory creation failed");

      server_respond(response_message);
    }
    else
    {
      // creation of directory successful
      printf("MD: Directory creation successful\n");

      strcat(response_message, "S:200 ");
      strcat(response_message, "Directory creation successful");

      server_respond(response_message);
    }
  }

  printf("COMMAND: MD complete\n\n");
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

#pragma endregion Commands

int server_listenForClients()
{
  if (listen(socket_desc, 1) < 0)
  {
    printf("ERROR: Error while listening\n");
    server_closeServerSocket();
    return -1;
  }
  printf("\nListening for incoming connections.....\n");

  // Accept an incoming connection:
  client_size = sizeof(client_addr);
  client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

  if (client_sock < 0)
  {
    printf("ERROR: Can't accept\n");
    server_closeServerSocket();
    return -1;
  }
  printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  return 0;
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
  int status;
  // Initialize server socket and bind to port:
  status = initServer();
  if (status != 0)
    return 0;

  // Listen for clients:
  status = server_listenForClients();
  if (status != 0)
    return 0;

  // Receive client's message:
  server_listenForCommand();

  // Closing server socket:
  server_closeServerSocket();

  return 0;
}