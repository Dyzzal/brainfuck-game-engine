#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct BfProgram{
    unsigned char arr[30000];
    int pntrPos;
} BfProgram;

typedef struct{
    FILE *bfFile;
    char *filename;
    unsigned char *buffer;
    
    unsigned char *file;
    bool fileMode;
    int fileLength;
    bool isFileLoading;
    
    char *openFileBuffer;
    FILE *openFile;
    int filePntrPos;
    
    int *loopStarts;
    int loops;
} BfScript;

int FindChar(char *str, char target, size_t start_index);

// Use to initialize a script
BfScript EmptyScript();

// Loads the bf program from the file into the buffer object in the BfScript
BfScript LoadFile(BfScript script, const char *filename);

// Executes the program stored in the script's buffer object. Returns 1 if successful
int ExecuteScript(BfProgram program, BfScript script);
