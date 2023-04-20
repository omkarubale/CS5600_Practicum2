#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"

void printCommandOutput(char command[200])
{

    char result[10000];
    FILE *fp;

    /* run command and capture output */
    fp = popen(command, "r");

    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }

    /* read the output and store it in 'result' */
    size_t len = 0;
    while (fgets(result + len, sizeof(result) - len, fp) != NULL)
    {
        len = strlen(result);
    }

    printf("Output: %s\n\n", result);

    /* close the file pointer */
    pclose(fp);
}

void displayLine()
{
    printf("-----------------------------\n\n");
}

int main()
{
    char command[200];
    printf("----------Starting Testing-----------\n\n");

    // GET : basic file
    printf("Test 1: Testing GET operation:\n");
    displayLine();

    sprintf(command, "./fget GET h3.txt f1/h2.txt");
    printCommandOutput(command);

    printf("Operation GET Successful!!\n");
    displayLine();

    // INFO: basic file
    printf("Test 2: Testing INFO of a file:\n");
    displayLine();

    sprintf(command, "./fget INFO h3.txt");
    printCommandOutput(command);

    printf("Operation INFO Successful!!\n");
    displayLine();

    // INFO: folder
    printf("Test 2.1: Testing INFO of a folder:\n");
    displayLine();

    sprintf(command, "./fget INFO ff");
    printCommandOutput(command);

    printf("Operation INFO Successful!!\n");
    displayLine();

    // MD: add newFolder
    printf("Test 3: Testing MD Command to create a new directory:\n");
    displayLine();

    sprintf(command, "./fget MD newFolder");
    printCommandOutput(command);

    printf("Operation MD Successful!!\n");
    displayLine();

    // PUT: smaller file TODO

    // PUT: large file
    printf("Test 4: Testing PUT operation LARGE FILE:\n");
    displayLine();

    sprintf(command, "./fget PUT lorem/loremContent.txt bigFile.txt");
    system(command);
    // printCommandOutput(command);

    printf("Operation PUT Successful!!\n");
    displayLine();

    // RM: remove newFolder
    printf("Test 5: Testing RM Command to remove a directory:\n");
    displayLine();

    sprintf(command, "./fget RM newFolder");
    printCommandOutput(command);

    printf("Operation RM Successful!!\n");
    displayLine();

    // RM: remove file TODO

    // Phase 2: Q6 - test cases demonstrates that mirrors work
    // How: rename folder for directory 1 to something different, trigger GET
    //      we will have active directory as Directory 2 now
    //      rename Directory 1 back to original (correct) name
    //      trigger GET command, and ensure CLONE SUCCESSFUL is found in result

    return 0;
}
