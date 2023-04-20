#include <stdbool.h>
#include <sys/time.h>

#pragma region Error and Success Codes

#define CODE_SIZE 5
#define CODE_PADDING 1

// Error codes
#define ERROR_NOT_FOUND "E:404"
#define ERROR_NOT_ACCEPTABLE "E:406"

// Success codes
#define SUCCESS_OK "S:200"
#define SUCCESS_CONTINUE "S:100"
#define SUCCESS_PARTIAL_CONTENT "S:206"

#define COMMAND_CODE_GET "C:001"
#define COMMAND_CODE_INFO "C:002"
#define COMMAND_CODE_PUT "C:003"
#define COMMAND_CODE_MD "C:004"
#define COMMAND_CODE_RM "C:005"

#pragma endregion Error and Success Codes

#pragma region Config

// Server config
#define SERVER_MESSAGE_SIZE 2000
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000

// client config
#define CLIENT_MESSAGE_SIZE 2000
#define CLIENT_COMMAND_SIZE 1000

#pragma endregion Config

typedef struct s_fileInfo
{
    char *name;
    int size;
    int permission;
    struct timeval lastAccessed;
} t_fileInfo;