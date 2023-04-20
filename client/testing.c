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
    printf("Test 6: Testing RM Command to remove a file:\n");
    displayLine();

    sprintf(command, "./fget RM filr.txt");
    printCommandOutput(command);

    printf("Operation RM Successful!!\n");
    displayLine();


    // Phase 2: Q6 - test cases demonstrates that mirrors work
    // How: rename folder for directory 1 to something different, trigger GET
    //      we will have active directory as Directory 2 now
    //      rename Directory 1 back to original (correct) name
    //      trigger GET command, and ensure "cloning complete" is found in result

    // changing the name of one of the copy of the server simulating that its not available
    system("mv ../server/root ../server/root1");

    // Triggering GET again: Note that only one of the directory is active now
    printf("Test 7: Testing GET operation with just one replica of server available:\n");
    printf("Note the server saying only Directory 1 is available");
    displayLine();

    sprintf(command, "./fget GET h5.txt f1/h2.txt");
    printCommandOutput(command);

    printf("Operation GET Successful!!\n");
    displayLine();

    // chnaging the name of the chnaged server space back to original, simulating that its now available.
    system("mv ../server/root1 ../server/root");

    // Triggering GET again: Note that both the replicas of the server are available now.
    printf("Test 8: Testing GET operation with just the second replica of server back up & available:\n");
    printf("Note the server displaying Cloning Complete.");
    displayLine();

    sprintf(command, "./fget GET h5.txt f1/h2.txt");
    printCommandOutput(command);

    printf("Operation GET Successful!!\n");
    displayLine();

    // Phase 3: Q7 - test cases demonstrate that multi-threading works
    // How: trigger reads in a for loop so we are sending more requests to the
    //      server than it can handle one request. Use big files for this so
    //      server waits for longer per request

    // Phase 4: Q8 - test cases demonstrate that distributed reads work
    // How: Can be same as Q7 testing if done well

    printf("Test 9: Triggering multiple reads/requests to the server showing multithreading");
    displayLine();

    //tests failing
    for ( int i = 0; i<20; i++) {
        sprintf(command, "./fget GET bigFile.txt f3/%d.txt",i);
        printCommandOutput(command);
    }

    printf("Distributed reads with multiprocessing tests done.");

    return 0;
}
