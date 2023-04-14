#include <stdbool.h>
#include <sys/time.h>

#pragma region Error and Success Codes

#define ERROR_CODE_SIZE 5
#define ERROR_CODE_PADDING 1

// Error codes
#define ERROR_NOT_FOUND "E:404"
#define ERROR_NOT_ACCEPTABLE "E:406"

// Success codes
#define SUCCESS_OK "S:200"
#define SUCCESS_CONTINUE "S:100"
#define SUCCESS_PARTIAL_CONTENT "S:206"

#pragma endregion Error and Success Codes

#pragma region Config

// Server config
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000
#define SERVER_MESSAGE_SIZE 2000

// client config
#define CLIENT_MESSAGE_SIZE 2000

#pragma endregion Config

typedef struct s_fileInfo
{
    char *name;
    int size;
    int permission;
    struct timeval lastAccessed;
} t_fileInfo;