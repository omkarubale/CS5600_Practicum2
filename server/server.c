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
#include <time.h>
#include <pthread.h>
#include "../common/common.h"
#include "configserver.h"

#define __USE_XOPEN_EXTENDED

int socket_desc;
struct sockaddr_in server_addr;

pthread_mutex_t command_mutex;

pthread_mutex_t root_directory_1_mutex;
pthread_mutex_t root_directory_2_mutex;

bool isRootDirectory1Init, isRootDirectory2Init;

/// @brief Closes the server socket.
void server_closeServerSocket()
{
  close(socket_desc);
  printf("EXIT: closed server socket\n");

  pthread_mutex_destroy(&command_mutex);
  printf("EXIT: destroyed command mutex\n");

  exit(1);
}

/// @brief Closes the client sockets connected to the server.
void server_closeClientSocket(int client_sock)
{
  close(client_sock);
}

#pragma region Directory Cloning

void directory_cloneDirectory2IntoDirectory1()
{
  char command[1000];
  memset(command, 0, sizeof(command));

  strcat(command, "cp -p -r ");
  strcat(command, ROOT_DIRECTORY_2);
  strcat(command, "*");
  strcat(command, " ");
  strcat(command, ROOT_DIRECTORY_1);

  pthread_mutex_lock(&root_directory_1_mutex);
  pthread_mutex_lock(&root_directory_2_mutex);

  printf("DIRECTORY CLONING: command to be excuted for clone root directory 2 into root directory 1: %s \n", command);
  printf("DIRECTORY CLONING: starting cloning root directory 2 into root directory 1\n");
  system(command);

  pthread_mutex_unlock(&root_directory_1_mutex);
  pthread_mutex_unlock(&root_directory_2_mutex);
  printf("DIRECTORY CLONING: cloning complete for root directory 2 into root directory 1\n");
}

void directory_cloneDirectory1IntoDirectory2()
{
  char command[1000];
  memset(command, 0, sizeof(command));

  strcat(command, "cp -p -r ");
  strcat(command, ROOT_DIRECTORY_1);
  strcat(command, "*");
  strcat(command, " ");
  strcat(command, ROOT_DIRECTORY_2);

  pthread_mutex_lock(&root_directory_1_mutex);
  pthread_mutex_lock(&root_directory_2_mutex);

  printf("DIRECTORY CLONING: command to be excuted for clone root directory 1 into root directory 2: %s \n", command);
  printf("DIRECTORY CLONING: starting cloning root directory 1 into root directory 2\n");
  system(command);

  pthread_mutex_unlock(&root_directory_1_mutex);
  pthread_mutex_unlock(&root_directory_2_mutex);
  printf("DIRECTORY CLONING: cloning complete for root directory 1 into root directory 2\n");
}

#pragma endregion Directory Cloning

#pragma region Init

/// @brief Initializes the socket when the server goes up.
/// @return 0 if socket creation is successful, -1 otherwise.
int init_createServerSocket()
{
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

/// @brief Binds the server socket to the port.
/// @return 0 in case of successful binding. -1 otherwise.
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

/// @brief Initialises a root directory, if not existing already, representing the server file space.
/// @return 0 if successful, -1 otherwise.
int init_createRootDirectory()
{
  struct stat st1 = {0};
  struct stat st2 = {0};

  bool isDirectory1Fresh = false;
  bool isDirectory2Fresh = false;

  // init mutex for directory 1
  if (pthread_mutex_init(&root_directory_1_mutex, NULL) != 0)
  {
    printf("INIT ERROR: root directory 1 mutex init failed\n");
    exit(1);
  }

  // init directory 1
  if (stat(ROOT_DIRECTORY_1, &st1) == -1)
  {
    // created using S_IREAD, S_IWRITE, S_IEXEC (0400 | 0200 | 0100) permission flags
    int res = mkdir(ROOT_DIRECTORY_1, 0700);
    if (res != 0)
    {
      isRootDirectory1Init = false;
      printf("INIT ERROR: root directory 1 creation failed\n");
    }
    else
    {
      isRootDirectory1Init = true;
      isDirectory1Fresh = true;
      printf("INIT: root directory 1 initialization successful\n");
    }
  }
  else
  {
    isRootDirectory1Init = true;
    isDirectory1Fresh = false;
    printf("INIT: root directory 1 already exists.\n");
  }

  // init mutex for directory 2
  if (pthread_mutex_init(&root_directory_2_mutex, NULL) != 0)
  {
    printf("INIT ERROR: root directory 2 mutex init failed\n");
    exit(1);
  }

  // init directory 2
  if (stat(ROOT_DIRECTORY_2, &st2) == -1)
  {
    // created using S_IREAD, S_IWRITE, S_IEXEC (0400 | 0200 | 0100) permission flags
    int res = mkdir(ROOT_DIRECTORY_2, 0700);
    if (res != 0)
    {
      isRootDirectory2Init = false;
      printf("INIT ERROR: root directory 2 creation failed\n");
    }
    else
    {
      isRootDirectory2Init = true;
      isDirectory2Fresh = true;
      printf("INIT: root directory 2 initialization successful\n");
    }
  }
  else
  {
    isRootDirectory2Init = true;
    isDirectory2Fresh = false;
    printf("INIT: root directory 2 already exists.\n");
  }

  if (!isRootDirectory1Init && !isRootDirectory1Init)
  {
    printf("INIT ERROR: both directory 1 and directory 2 init failed\n");
    return -1;
  }

  if (isRootDirectory1Init && isRootDirectory2Init)
  {
    if (!isDirectory1Fresh && !isDirectory2Fresh)
    {
      printf("INIT: both directory 1 and directory 2 already exist\n");
    }
    else if (isDirectory1Fresh && isDirectory2Fresh)
    {
      printf("INIT: both directory 1 and directory 2 are fresh\n");
    }
    else if (!isDirectory1Fresh && isDirectory2Fresh)
    {
      printf("INIT: directory 2 is fresh, cloning directory 1 into directory 2\n");
      directory_cloneDirectory1IntoDirectory2();
    }
    else if (isDirectory1Fresh && !isDirectory2Fresh)
    {
      printf("INIT: directory 1 is fresh, cloning directory 2 into directory 1\n");
      directory_cloneDirectory2IntoDirectory1();
    }
  }

  return 0;
}

int init_createCommandMutex()
{
  if (pthread_mutex_init(&command_mutex, NULL) != 0)
  {
    printf("INIT ERROR: output file mutex init failed\n");
    exit(1);
  }
  return 0;
}

/// @brief Initialises the server. Creates and binds socket to a port and creates root directory.
/// @return 0 if process is successful. -1 otherwise.
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
  status = init_createCommandMutex();
  if (status != 0)
    return -1;

  return 0;
}

#pragma endregion Init

#pragma region Directory Management

bool directory_isDirectory1Init()
{
  struct stat st = {0};

  if (isRootDirectory1Init)
  {
    // if directory 1 is marked as initialized, check whether it exists
    if (stat(ROOT_DIRECTORY_1, &st) == 0)
    {
      // it exists and is available
      printf("DIRECTORY: directory 1 is available\n");
      return true;
    }
    else
    {
      // directory was marked initialized but isn't available anymore
      isRootDirectory1Init = false;

      printf("DIRECTORY: directory 1 was marked initialized but isn't available anymore\n");
      return false;
    }
  }
  else
  {
    // directory was marked as not initialized, check whether it exists
    if (stat(ROOT_DIRECTORY_1, &st) == 0)
    {
      // it exists, need to be initialized again
      printf("DIRECTORY: Directory 1 is available now, cloning from directory 2\n");

      struct stat st2 = {0};

      if (isRootDirectory2Init && stat(ROOT_DIRECTORY_2, &st2) == 0)
      {
        // clone directory 2 into directory 1
        directory_cloneDirectory2IntoDirectory1();

        isRootDirectory1Init = true;

        return true;
      }
      else
      {
        // both directories are not available, and we cannot serve any requests
        printf("DIRECTORY ERROR: root directory 1 and root directory 2 both are unavailable\n");
        exit(1);
      }

      isRootDirectory1Init = false;

      return true;
    }
    else
    {
      // directory doesn't exist and is not available
      printf("DIRECTORY: directory 1 is not available\n");
      return false;
    }
  }
}

bool directory_isDirectory2Init()
{
  struct stat st = {0};

  if (isRootDirectory2Init)
  {
    // if directory 2 is marked as initialized, check whether it exists
    if (stat(ROOT_DIRECTORY_2, &st) == 0)
    {
      // it exists and is available
      printf("DIRECTORY: directory 2 is available\n");
      return true;
    }
    else
    {
      // directory was marked initialized but isn't available anymore
      isRootDirectory2Init = false;

      printf("DIRECTORY: directory 2 was marked initialized but isn't available anymore\n");
      return false;
    }
  }
  else
  {
    // directory was marked as not initialized, check whether it exists
    if (stat(ROOT_DIRECTORY_2, &st) == 0)
    {
      // it exists, need to be initialized again
      printf("DIRECTORY: Directory 2 is available now, cloning from directory 1\n");

      struct stat st1 = {0};

      if (isRootDirectory2Init && stat(ROOT_DIRECTORY_1, &st1) == 0)
      {
        // clone directory 1 into directory 2
        directory_cloneDirectory1IntoDirectory2();

        isRootDirectory2Init = true;

        return true;
      }
      else
      {
        // both directories are not available, and we cannot serve any requests
        printf("DIRECTORY ERROR: root directory 2 and root directory 1 both are unavailable\n");
        exit(1);
      }

      isRootDirectory2Init = false;

      return true;
    }
    else
    {
      // directory doesn't exist and is not available
      printf("DIRECTORY: directory 2 is not available\n");
      return false;
    }
  }
}

#pragma endregion Directory Management

#pragma region Communication

/// @brief Sends a message to the client.
/// @param client_sock is the socket of the client the message is to be sent to.
/// @param server_message represents the server message.
void server_sendMessageToClient(int client_sock, char *server_message)
{
  printf("SENDING TO CLIENT: %s\n", server_message);
  if (send(client_sock, server_message, strlen(server_message), 0) < 0)
  {
    printf("ERROR: Can't send\n");
    server_closeServerSocket();
  }
}

/// @brief Receives a message from the client.
/// @param client_sock is the socket of the client the message is to be received from.
/// @param client_message represents the received client message.
void server_recieveMessageFromClient(int client_sock, char *client_message)
{
  // Receive the server's response:
  if (recv(client_sock, client_message, CODE_SIZE + CODE_PADDING + CLIENT_MESSAGE_SIZE, 0) < 0)
  {
    printf("ERROR: Error while receiving client's msg\n");
    server_closeServerSocket();
  }

  printf("RECIEVED FROM CLIENT: %s\n", client_message);
}

#pragma endregion Communication

#pragma region Helpers

/// @brief Checks the existence of a directory in the server space.
/// @param path represents the directory path that needs to be examined for existence.
/// @return true if the directory exists, false otherwise.
bool isDirectoryExists(const char *path)
{
  struct stat stats;

  stat(path, &stats);

  // Check for file existence
  if (S_ISDIR(stats.st_mode))
    return true;

  return false;
}

/// @brief Checks the existence of a file.
/// @param filename represents the file path that needs to be checked.
/// @return true if the file exists, false otherwise.
bool isFileExists(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  bool is_exist = false;
  if (fp != NULL)
  {
    is_exist = true;
    fclose(fp); // close the file
  }
  return is_exist;
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

/// @brief To receive a file from client to the server.
/// @param remote_file_path represents the path in server space where the received file needs to be stored.
/// @param local_file_path is the path of original file in client space.
void command_get(int client_sock, char *remote_file_path, char *local_file_path)
{
  printf("COMMAND: GET started\n");

  pthread_mutex_lock(&command_mutex);

  FILE *remote_file;

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  char actual_path[200];

  if (directory_isDirectory1Init())
  {
    printf("GET: Directory 1 is available\n");
    strcpy(actual_path, ROOT_DIRECTORY_1);
  }
  else if (directory_isDirectory2Init())
  {
    printf("GET: Directory 2 is available\n");
    strcpy(actual_path, ROOT_DIRECTORY_2);
  }
  else
  {
    printf("GET ERROR: No Directory is available\n");
    server_closeServerSocket();
    exit(1);
  }

  strcat(actual_path, "/");
  strncat(actual_path, remote_file_path, strlen(remote_file_path));

  remote_file = fopen(actual_path, "r");
  printf("GET: Looking for file: %s\n", actual_path);

  // Check if the file exists on the server
  if (remote_file == NULL)
  {
    // file doesn't exist
    printf("GET ERROR: File not found on server\n");

    strcat(response_message, "E:404 ");
    strcat(response_message, "File not found on server");

    server_sendMessageToClient(client_sock, response_message);
  }
  else
  {
    printf("GET: File Found on server\n");

    // Send success response to client
    strcat(response_message, "S:200 ");
    strcat(response_message, "File found on server");

    server_sendMessageToClient(client_sock, response_message);
    memset(response_message, 0, sizeof(response_message));

    // recieve client's first response
    char client_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
    memset(client_message, '\0', sizeof(client_message));

    server_recieveMessageFromClient(client_sock, client_message);

    if (strncmp(client_message, "S:100", CODE_SIZE) == 0)
    {
      // Client said we can start sending the file
      // Send file data to client
      printf("GET: Client hinted at sending file contents.\n");
      char buffer[SERVER_MESSAGE_SIZE - 1];
      memset(buffer, 0, sizeof(buffer));
      int bytes_read;

      while (true)
      {
        if (strncmp(client_message, "S:100", CODE_SIZE) != 0)
        {
          printf("GET ERROR: stopped abruptly because client is not accepting data anymore\n");
          break;
        }

        if ((bytes_read = fread(buffer, sizeof(char), SERVER_MESSAGE_SIZE - 1, remote_file)) > 0)
        {
          printf("BUFFER: %s \n", buffer);
          memset(response_message, 0, sizeof(response_message));

          strcat(response_message, "S:206 ");
          strncat(response_message, buffer, bytes_read);

          memset(buffer, 0, sizeof(buffer));

          server_sendMessageToClient(client_sock, response_message);

          memset(client_message, '\0', sizeof(client_message));

          server_recieveMessageFromClient(client_sock, client_message);
        }
        else
        {
          printf("GET: reached end of file\n");

          memset(response_message, 0, sizeof(response_message));

          strcat(response_message, "S:200 ");
          strcat(response_message, "File sent successfully");

          server_sendMessageToClient(client_sock, response_message);
          break;
        }
      }
    }
    else
    {
      memset(response_message, 0, sizeof(response_message));

      strcat(response_message, "E:500 ");
      strcat(response_message, "The client did not agree to receive the file contents.");

      server_sendMessageToClient(client_sock, response_message);

      printf("GET: The client did not agree to receive the file contents.\n");
    }
  }

  fclose(remote_file);
  pthread_mutex_unlock(&command_mutex);

  printf("COMMAND: GET complete\n\n");
}

/// @brief
/// @param remote_file_path

/// @brief Gives the relevant information for a file.
/// @param client_sock is the socket of the client which is requesting the information.
/// @param remote_file_path is the path of the file whose information is requested.
void command_info(int client_sock, char *remote_file_path)
{
  printf("COMMAND: INFO started\n");

  pthread_mutex_lock(&command_mutex);

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY_1);
  strcat(actual_path, "/");
  strncat(actual_path, remote_file_path, strlen(remote_file_path));

  printf("INFO: actual path: %s\n", actual_path);

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  if (!isDirectoryExists(actual_path) && !isFileExists(actual_path))
  {
    // directory doesn't exist
    printf("INFO: Directory/File doesn't exist\n");

    strcat(response_message, "E:404 ");
    strcat(response_message, "Directory/File doesn't exist");

    server_sendMessageToClient(client_sock, response_message);
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

      server_sendMessageToClient(client_sock, response_message);
    }
    else
    {
      printf("INFO: Directory/File Information Retrieval successful\n");

      printf("TEST: %s", response_message);
      strcat(response_message, "S:200 Information Retrieval successful\n");
      printf("TEST2: %s", response_message);
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

      server_sendMessageToClient(client_sock, response_message);
    }
  }

  pthread_mutex_unlock(&command_mutex);

  printf("COMMAND: INFO complete\n\n");
}

/// @brief Creates a directory in the server.
/// @param client_sock represents the socket of the client that is requesting the command.
/// @param folder_path represents the path of the directory to be created.
void command_makeDirectory(int client_sock, char *folder_path)
{
  printf("COMMAND: MD started\n");

  pthread_mutex_lock(&command_mutex);

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY_1);
  strcat(actual_path, "/");
  strncat(actual_path, folder_path, strlen(folder_path));

  printf("MD: actual path: %s\n", actual_path);

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));

  if (isDirectoryExists(actual_path))
  {
    // directory already exists
    printf("MD: Directory already exists\n");

    strcat(response_message, "E:406 ");
    strcat(response_message, "Directory already exists");

    server_sendMessageToClient(client_sock, response_message);
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

      server_sendMessageToClient(client_sock, response_message);
    }
    else
    {
      // creation of directory successful
      printf("MD: Directory creation successful\n");

      strcat(response_message, "S:200 ");
      strcat(response_message, "Directory creation successful");

      server_sendMessageToClient(client_sock, response_message);
    }
  }

  pthread_mutex_unlock(&command_mutex);

  printf("COMMAND: MD complete\n\n");
}

/// @brief
/// @param local_file_path
/// @param remote_file_path

/// @brief To create and store a replica of a local client file to server space.
/// @param client_sock is the socket of the client that is requesting the command.
/// @param local_file_path is the path of the local file.
/// @param remote_file_path is the path in server where the replica needs to be saved.
void command_put(int client_sock, char *local_file_path, char *remote_file_path)
{
  printf("COMMAND: PUT started\n");

  pthread_mutex_lock(&command_mutex);

  // prepare the file to write into
  FILE *remote_file;
  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY_1);
  strcat(actual_path, "/");
  strncat(actual_path, remote_file_path, strlen(remote_file_path));

  remote_file = fopen(actual_path, "w");

  char response_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(response_message, 0, sizeof(response_message));
  char client_message[CODE_SIZE + CODE_PADDING + SERVER_MESSAGE_SIZE];
  memset(client_message, 0, sizeof(client_message));

  if (remote_file == NULL)
  {
    printf("PUT ERROR: File could not be opened. Please check whether the location exists.\n");

    strcat(response_message, "E:404 ");
    strcat(response_message, "File could not be opened. Please check whether the location exists.");

    server_sendMessageToClient(client_sock, response_message);
  }
  else
  {
    // Tell client that server is ready to recieve the file
    strcat(response_message, "S:100 ");
    strcat(response_message, "Ready to write file on server");

    server_sendMessageToClient(client_sock, response_message);

    // get first block from client
    server_recieveMessageFromClient(client_sock, client_message);

    while (true)
    {
      if (strncmp(client_message, "S:206", CODE_SIZE) == 0)
      {
        char *file_contents;
        file_contents = client_message + CODE_SIZE + CODE_PADDING;

        printf("PUT: Writing to file: %s\n", file_contents);

        fwrite(file_contents, sizeof(char), strlen(file_contents), remote_file);

        memset(response_message, 0, sizeof(response_message));
        strcat(response_message, "S:100 ");
        strcat(response_message, "Success Continue");

        server_sendMessageToClient(client_sock, response_message);

        // get next block from client
        memset(client_message, 0, sizeof(client_message));
        server_recieveMessageFromClient(client_sock, client_message);
      }
      else if (strncmp(client_message, "E:500", CODE_SIZE) == 0)
      {
        printf("PUT ERROR: File could not be recieved\n");

        break;
      }
      else if (strncmp(client_message, "S:200", CODE_SIZE) == 0)
      {
        memset(response_message, 0, sizeof(response_message));
        strcat(response_message, "S:200 ");
        strcat(response_message, "File received successfully");

        server_sendMessageToClient(client_sock, client_message);

        printf("PUT: File received successfully\n");

        break;
      }
    }

    fclose(remote_file);
  }

  pthread_mutex_unlock(&command_mutex);

  printf("COMMAND: PUT complete\n\n");
}

/// @brief Removes the indicated file/directory.
/// @param client_sock represents the socket of the client that is requesting the command
/// @param path represents the path of the file/directory to be removed.
void command_remove(int client_sock, char *path)
{
  printf("COMMAND: RM started\n");

  pthread_mutex_lock(&command_mutex);

  char actual_path[200];
  strcpy(actual_path, ROOT_DIRECTORY_1);
  strcat(actual_path, "/");
  strncat(actual_path, path, strlen(path));

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

    server_sendMessageToClient(client_sock, response_message);
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

        server_sendMessageToClient(client_sock, response_message);
      }
      else
      {
        // creation of directory successful
        printf("RM: File Removal successful\n");

        strcat(response_message, "S:200 ");
        strcat(response_message, "File Removal successful");

        server_sendMessageToClient(client_sock, response_message);
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

        server_sendMessageToClient(client_sock, response_message);
      }
      else
      {
        // creation of directory successful
        printf("RM: Directory Removal successful\n");

        strcat(response_message, "S:200 ");
        strcat(response_message, "Directory Removal successful");

        server_sendMessageToClient(client_sock, response_message);
      }
    }
    else
    {
      // Unsupported path
      printf("RM ERROR: Given path is not supported\n");
      strcat(response_message, "E:406 ");
      strcat(response_message, "Given path is not supported");

      server_sendMessageToClient(client_sock, response_message);
    }
  }

  pthread_mutex_unlock(&command_mutex);

  printf("COMMAND: RM complete\n\n");
}

#pragma endregion Commands

/// @brief Listens and server for incoming client connections.
/// @return 0 if slient connection to server is successful, -1 otherwise.
int server_listenForClients()
{
  if (listen(socket_desc, 1) < 0)
  {
    printf("ERROR: Error while listening\n");
    server_closeServerSocket();
    return -1;
  }
  printf("\nListening for incoming connections.....\n");

  socklen_t client_size;
  struct sockaddr_in client_addr;

  // Accept an incoming connection:
  client_size = sizeof(client_addr);
  int client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

  if (client_sock < 0)
  {
    printf("ERROR: Can't accept\n");
    server_closeServerSocket();
    return -1;
  }
  printf("CLIENT CONNECTION: Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  printf("CLIENT CONNECTION: Client socket: %d\n", client_sock);
  return client_sock;
}

/// @brief Listens for any incoming commands from the client, parses them and delegates the control to appropriate functions.
///          The server functions are not directly exposed to the client and all control passes through this method.
/// @param client_sock_arg represnts the socket of the incoming client for connection.
/// @return NULL when the server terminates.
void *server_listenForCommand(void *client_sock_arg)
{
  int client_sock = *((int *)client_sock_arg);
  free(client_sock_arg);

  char client_command[CLIENT_COMMAND_SIZE];
  memset(client_command, 0, sizeof(client_command));

  printf("LISTEN: listening for command from client socket: %d\n", client_sock);

  if (recv(client_sock, client_command, sizeof(client_command), 0) < 0)
  {
    printf("LISTEN ERROR: Couldn't listen for command\n");
    server_closeClientSocket(client_sock);

    return NULL;
  }

  printf("LISTEN: Message from client: %s\n", client_command);

  // Interpret entered command
  char *pch;
  pch = strtok(client_command, " \n");

  char *args[3];

  args[0] = pch;
  pch = strtok(NULL, " \n");

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
  else
  {
    printf("LISTEN ERROR: Invalid command provided\n");
    server_sendMessageToClient(client_sock, "E:404 Invalid command");
  }

  // Parse remaining arguements based on set command

  while (pch != NULL)
  {
    if (argc >= argcLimit)
    {
      printf("LISTEN ERROR: Invalid number of arguements provided\n");
    }

    args[argc++] = pch;
    pch = strtok(NULL, " \n");
  }

  // Redirect to correct command
  if (strcmp(args[0], "C:001") == 0)
  {
    command_get(client_sock, args[1], args[2]);
  }
  else if (strcmp(args[0], "C:002") == 0)
  {
    command_info(client_sock, args[1]);
  }
  else if (strcmp(args[0], "C:003") == 0)
  {
    command_put(client_sock, args[1], args[2]);
  }
  else if (strcmp(args[0], "C:004") == 0)
  {
    command_makeDirectory(client_sock, args[1]);
  }
  else if (strcmp(args[0], "C:005") == 0)
  {
    command_remove(client_sock, args[1]);
  }
  else
  {
    printf("LISTEN ERROR: Invalid command provided\n");
  }

  printf("LISTEN: Closing connection for client socket %d\n", client_sock);
  server_closeClientSocket(client_sock);

  return NULL;
}

int main(void)
{
  int status;
  // Initialize server socket and bind to port:
  status = initServer();
  if (status != 0)
    return 0;

  while (true)
  {
    // Listen for clients:
    int client_sock = server_listenForClients();
    if (client_sock < 0)
    {
      printf("CLIENT CONNECTION ERROR: Client could not be connected\n");
      continue;
    }

    // Create a new detached thread to serve client's message:
    pthread_t thread_for_client_request;
    pthread_attr_t attr;

    // detach thread so that it can end without having to join it here again
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int *arg = malloc(sizeof(*arg));
    if (arg == NULL)
    {
      fprintf(stderr, "CLIENT CONNECTION ERROR: Couldn't allocate memory for thread arguments.\n");
      exit(EXIT_FAILURE);
    }

    *arg = client_sock;

    // start listening for command from client
    pthread_create(&thread_for_client_request, NULL, server_listenForCommand, arg);
  }

  // Closing server socket:
  server_closeServerSocket();

  return 0;
}