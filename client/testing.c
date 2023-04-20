#include <stdlib.h>
#include <stdio.h>

void printCommandOutput(char command[100]) {

    char result[1024];
    FILE *fp;

    /* run command and capture output */
    fp = popen(command, "r");

    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* read the output and store it in 'result' */
    size_t len = 0;
    while (fgets(result+len, sizeof(result)-len, fp) != NULL) {
        len = strlen(result);
    }

    // printf("Output: %s\n\n", result, "\n");

    /* close the file pointer */
    pclose(fp);
}

void displayLine() {
    printf("-----------------------------\n\n");
}

int main() {
    char command[100];
    printf("----------Starting Testing-----------\n\n");

    printf("Test 1: Testing GET operation:\n");
    displayLine();

    // system("./fget GET file.txt f1/h2.txt");
    /* command to execute */
    sprintf(command, "./fget GET file.txt f1/h2.txt");
    printCommandOutput(command);

    printf("Operation GET Successful!!\n");
    displayLine();
    

    printf("Test 2: Testing INFO of a file:\n");
    displayLine();

    sprintf(command, "./fget INFO file.txt");
    printCommandOutput(command);

    printf("Operation INFO Successful!!\n");
    displayLine();


    printf("Test 3: Testing MD Command to create a new directory:\n");
    displayLine();

    sprintf(command, "./fget MD newFolder");
    printCommandOutput(command);

    printf("Operation MD Successful!!\n");
    displayLine();


    printf("Test 4: Testing RM Command to remove a directory:\n");
    displayLine();

    sprintf(command, "./fget RM newFolder");
    printCommandOutput(command);

    printf("Operation RM Successful!!\n");
    displayLine();


    printf("Test 5: Testing PUT operation LARGE FILE:\n");
    displayLine();

    sprintf(command, "./fget PUT f1/h1.txt bigFile.txt");
    printCommandOutput(command);

    printf("Operation PUT Successful!!\n");
    displayLine();

    return 0;
}

