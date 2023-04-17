/*
 * server.c -- TCP Socket Server
 *
 * adapted from:
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */
#define _XOPEN_SOURCE 500 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include "../common/common.h"

#define __USE_XOPEN_EXTENDED 

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

  // Bind to the set port and IP:S
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

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  int rv = remove(fpath);

  if (rv)
    perror(fpath);

  return rv;
}

int rmrf(char *path)
{
  return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

#pragma endregion Helpers

#pragma region Commands

/*
void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");
  char server_response[200];
  FILE *remote_file;
  
  char filename[200] =  "./root/try1/hello.txt";
  remote_file = fopen(filename, "r");
  perror("fopen");
  printf("Starting CHeck\n");
  // Check if the file exists on the server
  if ( remote_file == NULL)
  {
    printf("Error: File not found on server\n");
    snprintf(server_response, 1000, "E:404 File not found on server");
    send(client_sock, server_response, strlen(server_response), 0);
  }
  else {
    printf("File Found\n");
    // Send success response to client
    snprintf(server_response, 1000, "S:200 File found on server\n");
    send(client_sock, server_response, strlen(server_response), 0);
    
    recv(client_sock, client_command, sizeof(client_command), 0);
    if (strncmp(client_command, "S:100", CODE_SIZE) == 0) {
      // Send file data to client
      printf("Client hited at sending file contents.\n");
      char buffer[1000];
      int bytes_read;

      while ((bytes_read = fread(buffer, sizeof(char), 1000, remote_file)) > 0)
      {
        snprintf(server_response, 1000, "S:100 Data Incoming\n");
        send(client_sock, server_response, strlen(server_response), 0);

        recv(client_sock, client_command, sizeof(client_command), 0);
        printf("Client Command: %s \n", client_command);
        if (strncmp(client_command, "S:100", CODE_SIZE) == 0) {
          printf("BUFFER: %s \n", buffer);
          send(client_sock, buffer, bytes_read, 0);
        }
        else {
          printf("");
          snprintf(server_response, 1000, "S:404 Data Finished\n");
          send(client_sock, server_response, strlen(server_response), 0);
          printf("File sent successfully\n");
          break;
        }

        // printf("BUFFER: %s \n", buffer);
        // snprintf(server_response, 1000, "S:100 Data Incoming\n");
        // send(client_sock, server_response, strlen(server_response), 0);
        // send(client_sock, buffer, bytes_read, 0);
      }
    }
    else {
      printf("The client did not agree to receive the file contents.\n");
    }

  }

  fclose(remote_file);
  printf("COMMAND: GET complete\n");
}
*/
void command_get(char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");
  char server_response[200];
  FILE *remote_file;
  
  // char filename[200] =  "./root/try1/hello.txt";
  remote_file = fopen(remote_file_path, "r");
  perror("fopen");
  printf("Starting CHeck\n");
  // Check if the file exists on the server
  if ( remote_file == NULL)
  {
    printf("Error: File not found on server\n");
    snprintf(server_response, 1000, "E:404 File not found on server");
    send(client_sock, server_response, strlen(server_response), 0);
  }
  else {
    printf("File Found\n");
    // Send success response to client
    snprintf(server_response, 1000, "S:200 File found on server\n");
    send(client_sock, server_response, strlen(server_response), 0);
    
    recv(client_sock, client_command, sizeof(client_command), 0);
    if (strncmp(client_command, "S:100", CODE_SIZE) == 0) {
      // Send file data to client
      printf("Client hited at sending file contents.\n");
      char buffer[1000];
      int bytes_read;

      if ((bytes_read = fread(buffer, sizeof(char), 1000, remote_file)) > 0)
      {
        printf("BUFFER: %s \n", buffer);
        send(client_sock, buffer, bytes_read, 0);
      }

      snprintf(server_response, 0, "");
      send(client_sock, server_response, strlen(server_response), 0);

      printf("File sent successfully\n");
    }
    else {
      printf("The client did not agree to receive the file contents.\n");
    }

  }

  fclose(remote_file);
  printf("COMMAND: GET complete\n");
}

void command_info(char *remote_file_path)
{
  printf("COMMAND: INFO started\n");

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY);
  strcat(actual_path, "/");
  strncat(actual_path, remote_file_path, strlen(remote_file_path) - 1);

  printf("INFO: actual path: %s\n", actual_path);

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  if (!isDirectoryExists(actual_path))
  {
    // directory doesn't exist
    printf("INFO: Directory/File doesn't exist\n");

    strcat(response_message, "E:404 ");
    strcat(response_message, "Directory/File doesn't exist");

    server_respond(response_message);
  }
  else
  {
    // directory exists
    printf("INFO: Directory/File exists\n");

    struct stat sb;

    if (stat(actual_path, &sb) == -1)
    {
      printf("INFO ERROR: Directory/File Information Retrieval failed\n");
      perror("stat");

      strcat(response_message, "E:406 ");
      strcat(response_message, "Directory/File Information Retrieval failed");

      server_respond(response_message);
    }
    else
    {
      printf("INFO: Directory/File Information Retrieval successful\n");

      strcat(response_message, "S:200 Information Retrieval successful\n");
      char temp[2000];
      memset(temp, '\0', sizeof(temp));

      sprintf(temp, "Ownership:                UID=%ld   GID=%ld\n", (long)sb.st_uid, (long)sb.st_gid);
      strcat(response_message, temp);
      sprintf(temp, "File size:                %lld bytes\n", (long long)sb.st_size);
      strcat(response_message, temp);
      sprintf(temp, "Last file access:         %s", ctime(&sb.st_atime));
      strcat(response_message, temp);
      sprintf(temp, "Last file modification:   %s", ctime(&sb.st_mtime));
      strcat(response_message, temp);

      server_respond(response_message);
    }
  }

  printf("COMMAND: INFO complete\n");
}

void command_makeDirectory(char *folder_path)
{
  printf("COMMAND: MD started\n");

  char actual_path[200];
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

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY);
  strcat(actual_path, "/");
  strncat(actual_path, path, strlen(path) - 1);

  printf("RM: actual path: %s\n", actual_path);

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  struct stat sb;

  if (stat(actual_path, &sb) == -1)
  {
    printf("RM ERROR: Directory/File Not Found\n");
    perror("stat");

    strcat(response_message, "E:404 ");
    strcat(response_message, "Directory/File Not Found");

    server_respond(response_message);
  }
  else
  {

    if (S_ISREG(sb.st_mode))
    {
      // Path is a regular file
      int res = remove(actual_path);

      if (res != 0)
      {
        // creation of directory failed
        printf("RM ERROR: File Removal failed\n");
        perror("remove");
        strcat(response_message, "E:406 ");
        strcat(response_message, "File Removal failed");

        server_respond(response_message);
      }
      else
      {
        // creation of directory successful
        printf("RM: File Removal successful\n");

        strcat(response_message, "S:200 ");
        strcat(response_message, "File Removal successful");

        server_respond(response_message);
      }
    }
    else if (S_ISDIR(sb.st_mode))
    {
      // Path is a directory
      int res = rmrf(actual_path);

      if (res != 0)
      {
        // creation of directory failed
        printf("RM ERROR: Directory Removal failed\n");
        perror("remove");
        strcat(response_message, "E:406 ");
        strcat(response_message, "Directory Removal failed");

        server_respond(response_message);
      }
      else
      {
        // creation of directory successful
        printf("RM: Directory Removal successful\n");

        strcat(response_message, "S:200 ");
        strcat(response_message, "Directory Removal successful");

        server_respond(response_message);
      }
    }
    else
    {
      // Unsupported path
      printf("RM ERROR: Given path is not supported\n");
      strcat(response_message, "E:406 ");
      strcat(response_message, "Given path is not supported");
    }
  }

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

  #if defined __STDC_VERSION__  
    long version = __STDC_VERSION__;   
    if ( version == 199901 ) { printf ("version detected : C99\n"); }    
    if ( version == 201112 ) { printf ("version detected : C11\n"); }    
    if ( version == 201710 ) { printf ("version detected : C18\n"); } 
  #else 
    printf ("version detected : C90\n");
  #endif

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