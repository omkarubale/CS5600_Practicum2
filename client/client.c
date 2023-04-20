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

#define ROOT_DIRECTORY "./root"

int socket_desc;
struct sockaddr_in server_addr;

/// @brief  Closes the open socket for the client.
void client_closeClientSocket()
{
  // Close the socket:
  close(socket_desc);
  printf("EXIT: closing client socket\n");
  exit(1);
}

#pragma region Init

/// @brief Creates the socket for the client.
void init_createClientSocket()
{
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("ERROR: Unable to create socket\n");
    client_closeClientSocket();
  }

  printf("INIT: Socket created successfully\n");
}

/// @brief Initializes the client socket.
void init_initClient()
{
  init_createClientSocket();
}

/// @brief The client makes connection to the server.
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

#pragma endregion Init

#pragma region Communication

/// @brief To send message to the server.
/// @param client_message represents the message to be sent.
void client_sendMessageToServer(char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE])
{
  printf("SENDING TO SERVER: %s \n", client_message);

  // Send the message to server:
  if (send(socket_desc, client_message, strlen(client_message), 0) < 0)
  {
    printf("ERROR: Unable to send message \n");
    client_closeClientSocket();
  }
}

/// @brief To receive a message from the server.
/// @param server_message represents the received message.
void client_recieveMessageFromServer(char *server_message)
{
  // Receive the server's response:
  if (recv(socket_desc, server_message, CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE, 0) < 0)
  {
    printf("ERROR: Error while receiving server's message \n");
    client_closeClientSocket();
  }

  printf("RECIEVED FROM SERVER: %s \n", server_message);
}

#pragma endregion Communication

#pragma region Commands

/// @brief To get a file data from server to the local client space.
/// @param remote_file_path is the path of the remote file on server to be retrieved.
/// @param local_file_path is the file path where the data needs ro be stored in client.
void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");

  // Open local file for writing
  FILE *local_file;
  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY);
  strncat(actual_path, local_file_path, strlen(local_file_path));

  local_file = fopen(actual_path, "w");

  if (local_file == NULL)
  {
    printf("GET ERROR: Local file could not be opened. Please check whether the location exists.\n");
  }
  else
  {
    // Connect to server socket:
    client_connect();

    char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
    memset(client_message, 0, sizeof(client_message));
    char server_response[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
    memset(server_response, 0, sizeof(server_response));

    // sending message to server
    char code[CODE_SIZE + CODE_PADDING] = "C:001 ";
    strncat(client_message, code, CODE_SIZE + CODE_PADDING);

    strncat(client_message, remote_file_path, strlen(remote_file_path));
    strncat(client_message, " ", 1);
    strncat(client_message, local_file_path, strlen(local_file_path));

    client_sendMessageToServer(client_message);

    // Receive server response
    client_recieveMessageFromServer(server_response);

    // Check if the file exists on the server
    if (strncmp(server_response, "S:200", CODE_SIZE) == 0)
    {
      printf("GET: File Found - Server Response: %s \n", server_response);

      // Hint the server to send the file data requested
      memset(client_message, 0, sizeof(client_message));
      strcat(client_message, "S:100 ");
      strcat(client_message, "Success Continue");

      client_sendMessageToServer(client_message);

      // Receive file data from server and write it to local file
      // get first block from server
      memset(server_response, 0, sizeof(server_response));
      client_recieveMessageFromServer(server_response);

      // continue taking blocks from server until it is done
      while (true)
      {
        if (strncmp(server_response, "S:206", CODE_SIZE) == 0)
        {
          char *file_contents;
          file_contents = server_response + CODE_SIZE + CODE_PADDING;

          printf("GET: Writing to file: %s\n", file_contents);

          fwrite(file_contents, sizeof(char), strlen(file_contents), local_file);

          memset(client_message, 0, sizeof(client_message));
          strcat(client_message, "S:100 ");
          strcat(client_message, "Success Continue");

          client_sendMessageToServer(client_message);

          // get next block from server
          memset(server_response, 0, sizeof(server_response));
          client_recieveMessageFromServer(server_response);
        }
        else if (strncmp(server_response, "E:500", CODE_SIZE) == 0)
        {
          printf("GET ERROR: File could not be recieved\n");

          break;
        }
        else if (strncmp(server_response, "S:200", CODE_SIZE) == 0)
        {
          printf("GET: File received successfully\n");

          break;
        }
      }

      fclose(local_file);
    }
    else
    {
      printf("GET: File Not Found - Server Response: %s \n", server_response);
    }
  }

  printf("COMMAND: GET complete\n\n");
}

/// @brief To retreive relevant information for the file.
/// @param remote_file_path is the path of the file whose info is requested.
void command_info(char *remote_file_path)
{
  printf("COMMAND: INFO started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
  char server_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];

  // empty string init
  memset(client_message, '\0', sizeof(client_message));
  memset(server_message, '\0', sizeof(server_message));

  // build command to send to server
  char code[CODE_SIZE + CODE_PADDING] = "C:002 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);
  strncat(client_message, remote_file_path, strlen(remote_file_path));

  // Connect to server socket:
  client_connect();

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveMessageFromServer(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: INFO complete\n\n");
}

/// @brief To create and store a replica of a local client file to server space.
/// @param local_file_path is the path of the local file.
/// @param remote_file_path is the path in server where the replica needs to be saved.
void command_put(char *local_file_path, char *remote_file_path)
{
  printf("COMMAND: PUT started\n");

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY);
  strncat(actual_path, local_file_path, strlen(local_file_path));

  FILE *local_file;

  local_file = fopen(actual_path, "r");
  printf("PUT: Looking for file: %s\n", actual_path);

  if (local_file == NULL)
  {
    // file doesn't exist on client
    printf("PUT ERROR: File not found on client\n");
  }
  else
  {
    printf("PUT: File Found on client\n");

    // Connect to server socket:
    client_connect();

    char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
    memset(client_message, 0, sizeof(client_message));
    char server_response[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
    memset(server_response, 0, sizeof(server_response));

    // sending message to server
    char code[CODE_SIZE + CODE_PADDING] = "C:003 ";
    strncat(client_message, code, CODE_SIZE + CODE_PADDING);

    strncat(client_message, local_file_path, strlen(local_file_path));
    strncat(client_message, " ", 1);
    strncat(client_message, remote_file_path, strlen(remote_file_path));

    client_sendMessageToServer(client_message);

    // Receive server response
    client_recieveMessageFromServer(server_response);

    if (strncmp(server_response, "S:100", CODE_SIZE) == 0)
    {
      // Server is ready to recieve file contents. Start sending file
      printf("PUT: Server hinted at accepting file contents.\n");
      char buffer[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE - 1];
      int bytes_read;

      while (true)
      {
        if (strncmp(server_response, "S:100", CODE_SIZE) != 0)
        {
          printf("GET ERROR: stopped abruptly because server is not accepting data anymore\n");
          break;
        }

        if ((bytes_read = fread(buffer, sizeof(char), CLIENT_MESSAGE_SIZE - 1, local_file)) > 0)
        {
          printf("BUFFER: %s \n", buffer);

          memset(client_message, 0, sizeof(client_message));

          strcat(client_message, "S:206 ");
          strncat(client_message, buffer, bytes_read);

          client_sendMessageToServer(client_message);

          memset(server_response, '\0', sizeof(server_response));
          client_recieveMessageFromServer(server_response);
        }
        else
        {
          printf("PUT: reached end of file\n");

          memset(client_message, 0, sizeof(client_message));

          strcat(client_message, "S:200 ");
          strcat(client_message, "File sent successfully");

          client_sendMessageToServer(client_message);

          memset(server_response, '\0', sizeof(server_response));

          client_recieveMessageFromServer(server_response);

          if (strncmp(server_response, "S:200", CODE_SIZE) == 0)
          {
            printf("PUT: Server received file successfully\n");
          }
          else
          {
            printf("PUT ERROR: Server did not recieve file successfully\n");
          }
          break;
        }
      }
    }
    else
    {

      printf("PUT ERROR: The server did not agree to receive the file contents.\n");
    }
  }

  printf("COMMAND: PUT complete\n\n");
}

/// @brief Creates a folder on the indicated path.
/// @param folder_path represents the folder path that needs ro be created.
void command_makeDirectory(char *folder_path)
{
  printf("COMMAND: MD started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
  char server_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];

  // empty string init
  memset(client_message, '\0', sizeof(client_message));
  memset(server_message, '\0', sizeof(server_message));

  // build command to send to server
  char code[CODE_SIZE + CODE_PADDING] = "C:004 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);
  strncat(client_message, folder_path, strlen(folder_path));

  // Connect to server socket:
  client_connect();

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveMessageFromServer(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: MD complete\n\n");
}

/// @brief Removes a file or a directory.
/// @param path represents the path of the object that needs to be removed
void command_remove(char *path)
{
  printf("COMMAND: RM started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];
  char server_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];

  // empty string init
  memset(client_message, '\0', sizeof(client_message));
  memset(server_message, '\0', sizeof(server_message));

  // build command to send to server
  char code[CODE_SIZE + CODE_PADDING] = "C:005 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);
  strncat(client_message, path, strlen(path));

  // Connect to server socket:
  client_connect();

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveMessageFromServer(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: RM complete\n\n");
}

#pragma endregion Commands

/// @brief The communication between our server and client is via well defined protocols. This method acts as a
///         gateway for interaction and validation of requested commands.
/// @param argsCount represents the no. of arguments in the command.
/// @param argv represents the arguments in the command.
void client_parseCommand(int argsCount, char **argv)
{
  // Redirect to correct command
  if (strcmp(argv[1], "GET") == 0)
  {
    if (argsCount == 3)
    {
      command_get(argv[2], argv[2]);
    }
    else if (argsCount == 4)
    {
      command_get(argv[2], argv[3]);
    }
    else
    {
      printf("ERROR: Invalid number of arguements provided\n");
    }
  }
  else if (strcmp(argv[1], "INFO") == 0)
  {
    if (argsCount == 3)
    {
      command_info(argv[2]);
    }
    else
    {
      printf("ERROR: Invalid number of arguements provided\n");
    }
  }
  else if (strcmp(argv[1], "PUT") == 0)
  {
    if (argsCount == 3)
    {
      command_put(argv[2], argv[2]);
    }
    else if (argsCount == 4)
    {
      command_put(argv[2], argv[3]);
    }
    else
    {
      printf("ERROR: Invalid number of arguements provided\n");
    }
  }
  else if (strcmp(argv[1], "MD") == 0)
  {
    if (argsCount == 3)
    {
      if (argv[2][0] == '.' || argv[2][0] == '/')
      {
        printf("ERROR: Invalid arguements provided\n");
      }
      command_makeDirectory(argv[2]);
    }
    else
    {
      printf("ERROR: Invalid number of arguements provided\n");
    }
  }
  else if (strcmp(argv[1], "RM") == 0)
  {
    if (argsCount == 3)
    {
      if (argv[2][0] == '.' || argv[2][0] == '/')
      {
        printf("ERROR: Invalid arguements provided\n");
      }
      command_remove(argv[2]);
    }
    else
    {
      printf("ERROR: Invalid number of arguements provided\n");
    }
  }
  else
  {
    printf("ERROR: Invalid command provided\n");
  }
}

int main(int argc, char **argv)
{
  if (argc > 4)
  {
    printf("Incorrect number of arguements supplied\n");
    return 0;
  }

  if (strcmp(argv[1], "GET") != 0 &&
      strcmp(argv[1], "INFO") != 0 &&
      strcmp(argv[1], "PUT") != 0 &&
      strcmp(argv[1], "MD") != 0 &&
      strcmp(argv[1], "RM") != 0)
  {
    printf("Incorrect command provided!: %s\n", argv[1]);
    return 0;
  }

  // Initialize client socket:
  init_initClient();

  // Get text message to send to server:
  client_parseCommand(argc, argv);

  // Closing client socket:
  client_closeClientSocket();

  return 0;
}