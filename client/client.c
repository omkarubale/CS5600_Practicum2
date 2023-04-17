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

void client_closeClientSocket()
{
  // Close the socket:
  close(socket_desc);
  printf("EXIT: closing client socket\n");
  exit(1);
}

#pragma region Init

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

#pragma endregion Init

#pragma region Communication

void client_sendMessageToServer(char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE])
{
  // Send the message to server:
  if (send(socket_desc, client_message, strlen(client_message), 0) < 0)
  {
    printf("ERROR: Unable to send message\n");
    client_closeClientSocket();
  }
}

void client_recieveResponse(char *server_message)
{
  // Receive the server's response:
  if (recv(socket_desc, server_message, CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE, 0) < 0)
  {
    printf("ERROR: Error while receiving server's msg\n");
    client_closeClientSocket();
  }

  printf("RESPONSE: %s\n", server_message);
}

#pragma endregion Communication

#pragma region Commands

/*
void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];

  // empty string init
  memset(client_message, 0, sizeof(client_message));

  char code[CODE_SIZE + CODE_PADDING] = "C:001 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);

  strncat(client_message, remote_file_path, strlen(remote_file_path));
  strncat(client_message, " ", 1);
  strncat(client_message, local_file_path, strlen(local_file_path));

  client_sendMessageToServer(client_message);

  char server_response[10000];
  FILE *local_file;

  // Receive server response
  int bytes_received = recv(socket_desc, server_response, 1000, 0);
  if (bytes_received < 0)
  {
    perror("Error receiving server response");
    exit(EXIT_FAILURE);
  }
  printf("SR: %s\n", server_response);

  // Check if the file exists on the server
  if (strncmp(server_response, "S:200", CODE_SIZE) == 0)
  {
    printf("Server Response: %s \n", server_response);

    // Hint the server to send the file data requested
    // snprintf(client_message, 1000, "S:100 Success Continue\n");
    // send(socket_desc, client_message, strlen(client_message), 0);

    // Open local file for writing
    local_file = fopen(local_file_path, "w");
    // fwrite(server_response, sizeof(char), 1000, local_file);
    // Receive file data from server and write it to local file
    char buffer_message[10000];
    char buffer_data[10000];
    int bytes_received_message;
    int bytes_received_data;
    printf("HERE 2 \n");

    
    while (1)
    {
      printf("INSIDE WHILE");

      bytes_received_message = recv(socket_desc, buffer_message, 1000, 0);
      printf("Message Received: %s \n", buffer_message);

      if (strncmp(bytes_received_message, "S:100", CODE_SIZE) == 0) {

        snprintf(client_message, 1000, "S:100 Success Continue Sending Data\n");
        send(socket_desc, client_message, strlen(client_message), 0);

        bytes_received_data = recv(socket_desc, buffer_data, 1000, 0);
        fwrite(buffer_data, sizeof(char), bytes_received_data, local_file);

      }
      else {
        // break;
        fclose(local_file);

        printf("File received successfully\n");
      }
      // if (bytes_received <= 0)
      // {
      //   break;
      // }
      // fprintf(local_file, "%s", buffer);
      // fwrite(buffer, sizeof(char), bytes_received, local_file);
      // bzero(buffer, bytes_received);
    }
    
  while (1)
  {
    
  }
  

    fclose(local_file);

    printf("File received successfully\n");
  }
  else
  {
    printf("RESPONSE Error: File not found on server\n");
  }


  printf("COMMAND: GET complete\n");
}
*/
void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];

  // empty string init
  memset(client_message, 0, sizeof(client_message));

  char code[CODE_SIZE + CODE_PADDING] = "C:001 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);

  strncat(client_message, remote_file_path, strlen(remote_file_path));
  strncat(client_message, " ", 1);
  strncat(client_message, local_file_path, strlen(local_file_path));

  client_sendMessageToServer(client_message);

  char server_response[10000];
  FILE *local_file;

  // Receive server response
  int bytes_received = recv(socket_desc, server_response, 1000, 0);
  if (bytes_received < 0)
  {
    perror("Error receiving server response");
    exit(EXIT_FAILURE);
  }
  printf("SR: %s\n", server_response);
  printf("SR 2: %s\n", server_response);
  // Check if the file exists on the server
  if (strncmp(server_response, "S:200", CODE_SIZE) == 0)
  {
    printf("Server Response: %s \n", server_response);

    // Hint the server to send the file data requested
    snprintf(client_message, 1000, "S:100 Success Continue\n");
    send(socket_desc, client_message, strlen(client_message), 0);

    // Open local file for writing
    local_file = fopen(local_file_path, "w");
    // fwrite(server_response, sizeof(char), 1000, local_file);
    // Receive file data from server and write it to local file
    char buffer[10000];
    int bytes_received;
    printf("HERE 2 \n");

    if (1)
    {
      bytes_received = recv(socket_desc, buffer, 1000, 0);
      if (bytes_received <= 0)
      {
        printf("ZERO BYTE BLOCK");
      }
      fwrite(buffer, sizeof(char), bytes_received, local_file);
    }


    // int n;
    // while(1) {
    //   n = recv(socket_desc, buffer, 1000, 0);
    //   if (n<= 0) {
    //     break;
    //   }
    //   fprintf(local_file, "%s", buffer);
    //   bzero(buffer, 1000);
    // }

    fclose(local_file);

    printf("File received successfully\n");
  }
  else
  {
    printf("RESPONSE Error: File not found on server\n");
  }


  printf("COMMAND: GET complete\n");
} 

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

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveResponse(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: INFO complete\n");
}

void command_put(char *local_file_path, char *remote_file_path)
{
  printf("COMMAND: PUT started\n");

  char client_message[CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE];

  // empty string init
  memset(client_message, 0, sizeof(client_message));

  char code[CODE_SIZE + CODE_PADDING] = "C:003 ";
  strncat(client_message, code, CODE_SIZE + CODE_PADDING);

  strncat(client_message, local_file_path, strlen(local_file_path));
  strncat(client_message, " ", 1);
  strncat(client_message, remote_file_path, strlen(remote_file_path));

  client_sendMessageToServer(client_message);

  // TODO

  printf("COMMAND: PUT complete\n");
}

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

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveResponse(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: MD complete\n\n");
}

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

  // send command to server
  client_sendMessageToServer(client_message);

  // get response from server
  client_recieveResponse(server_message);

  memset(server_message, 0, sizeof(server_message));

  printf("COMMAND: RM complete\n");
}

#pragma endregion Commands

void client_getCommand()
{
  while (true)
  {
    char client_command[CLIENT_COMMAND_SIZE];
    memset(client_command, 0, sizeof(client_command));

    // Get input from the user:
    printf("Enter command: ");
    if (fgets(client_command, sizeof(client_command), stdin))
    {
      // Interpret entered command
      char *pch;
      pch = strtok(client_command, " ");

      char *args[3];
      args[0] = (char*) malloc(sizeof(char)*200);
      args[1] = (char*) malloc(sizeof(char)*200);
      args[2] = (char*) malloc(sizeof(char)*200);
      args[0] = pch;
      pch = strtok(NULL, " ");

      // Set arguement Limits based on first argument
      int argcLimit;
      int argc = 1;

      if (strcmp(args[0], "GET") == 0)
      {
        argcLimit = 3;
      }
      else if (strcmp(args[0], "INFO") == 0)
      {
        argcLimit = 2;
      }
      else if (strcmp(args[0], "PUT") == 0)
      {
        argcLimit = 3;
      }
      else if (strcmp(args[0], "MD") == 0)
      {
        argcLimit = 2;
      }
      else if (strcmp(args[0], "RM") == 0)
      {
        argcLimit = 2;
      }
      else if (strcmp(args[0], "Q") == 0 || strcmp(args[0], "Q\n") == 0)
      {
        argcLimit = 1;
      }
      else
      {
        printf("ERROR: Invalid command provided\n");
        continue;
      }

      // Parse remaining arguements based on set command
      bool errorsInTokenization = false;
      while (pch != NULL && argc < argcLimit)
      {
        if (argc >= argcLimit)
        {
          printf("ERROR: Invalid number of arguements provided\n");
          pch = NULL;
          errorsInTokenization = true;
          break;
        }

        args[argc++] = pch;
        pch = strtok(NULL, " ");
      }

      if (errorsInTokenization)
      {
        continue;
      }

      // Redirect to correct command
      if (strcmp(args[0], "GET") == 0)
      {
        if (strlen(args[1]) > 0 && strcmp(args[1], "\n") != 0)
        {
          if (strlen(args[2]) > 0 && strcmp(args[2], "\n") != 0) {
            command_get(args[1], args[2]);
          }
          else {
            command_get(args[1], args[1]);
          }
          
        }
        else
        {
          printf("ERROR: Invalid number of arguements provided1\n");
          continue;
        }
      }
      else if (strcmp(args[0], "INFO") == 0)
      {
        if (strlen(args[1]) > 0 && strcmp(args[1], "\n") != 0)
        {
          command_info(args[1]);
        }
        else
        {
          printf("ERROR: Invalid number of arguements provided\n");
          continue;
        }
      }
      else if (strcmp(args[0], "PUT") == 0)
      {
        if (strlen(args[1]) > 0 && strcmp(args[1], "\n") != 0 )
        {
          if (strlen(args[2]) > 0 && strcmp(args[2], "\n") != 0) {
            command_put(args[1], args[2]);
          }
          else {
            command_put(args[1], args[1]);
          }
        }
        else
        {
          printf("ERROR: Invalid number of arguements provided\n");
          continue;
        }
      }
      else if (strcmp(args[0], "MD") == 0)
      {
        if (strlen(args[1]) > 0 && strcmp(args[1], "\n") != 0)
        {
          command_makeDirectory(args[1]);
        }
        else
        {
          printf("ERROR: Invalid number of arguements provided\n");
          continue;
        }
      }
      else if (strcmp(args[0], "RM") == 0)
      {
        if (strlen(args[1]) > 0 && strcmp(args[1], "\n") != 0)
        {
          if (args[1][0] == '.' || args[1][0] == '/')
          {
            printf("ERROR: Invalid arguements for RM provided\n");
            continue;
          }
          command_remove(args[1]);
        }
        else
        {
          printf("ERROR: Invalid number of arguements provided\n");
          continue;
        }
      }
      else if (strcmp(args[0], "Q") == 0 || strcmp(args[0], "Q\n") == 0)
      {
        printf("CLIENT: quiting client\n");
        client_sendMessageToServer("C:999");
        return;
      }
      else
      {
        printf("ERROR: Invalid command provided\n");
        continue;
      }
    }
  }
}

int main(void)
{
  // Initialize client socket:
  init_initClient();

  // Connect to server socket:
  client_connect();

  // Get text message to send to server:
  client_getCommand();

  // Closing client socket:
  client_closeClientSocket();

  return 0;
}