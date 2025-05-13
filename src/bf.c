#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bf.h"

int FindChar(char *str, char target, size_t start_index) {
    if (str == NULL) {
        return -1;
    }

    size_t i = start_index;
    while (str[i] != '\0') {
        if (str[i] == target) {
            return i;
        }
        i++;
    }

    return i;
}

BfScript EmptyScript(){
    BfScript script;
    //program.arr = malloc(sizeof(char)*30000);
    script.bfFile = 0;
    script.buffer = NULL;//malloc(sizeof(char));
    script.file = NULL;//malloc(sizeof(char));
    script.fileMode = false;
    script.fileLength = 0;
    script.isFileLoading = false;
    script.filePntrPos = 0;
    script.loopStarts = NULL;//malloc(sizeof(int));
    script.loops = 0;
}

BfScript LoadFile(BfScript script, const char *filename){
    // Load file
    script.bfFile = fopen(filename, "r");
    if (script.bfFile == NULL) {
        perror("Error opening file");
        //return NULL;
    }
    script.filename = (char*)filename;
    
    // Get file size for buffer
    fseek(script.bfFile, 0, SEEK_END);
    long file_size = ftell(script.bfFile);
    rewind(script.bfFile);
    
    // Allocate memory to buffer
    script.buffer = (unsigned char*)malloc(sizeof(unsigned char)*(file_size+1));
    if (script.buffer == NULL) {
        perror("Error allocating memory");
        fclose(script.bfFile);
        //return 1;
    }
    
    // Read file to buffer
    size_t bytes_read = fread(script.buffer, 1, file_size, script.bfFile);
    if(bytes_read != file_size){
        perror("Error reading file");
        fclose(script.bfFile);
        free(script.buffer);
        //return 1;
    }
    
    // Add null-terminator to the end of the buffer
    script.buffer[file_size] = '\0';
    
    return script;
}

int ExecuteScript(BfProgram program, BfScript script){
    script.file = (unsigned char*)malloc(sizeof(char));
    
    script.loopStarts = malloc(sizeof(int));
    
    if (script.buffer == NULL) {
        perror("Buffer has no memory");
        free(script.buffer);
        free(script.loopStarts);
        return 1;
    }
    
    for(int i = 0; i < strlen(script.buffer); ++i){
        //if(script.openFile == NULL){
        //    perror("Error opening file");
        //}
        //steps++;
        unsigned char cmd = script.buffer[i];
        switch (cmd){
            case '>':
                program.pntrPos++;
                if(program.pntrPos > sizeof(program.arr)){
                    program.pntrPos -= sizeof(program.arr);
                }
                break;
            case '<':
                program.pntrPos--;
                if(program.pntrPos < 0){
                    program.pntrPos += sizeof(program.arr);
                }
                break;
            case '^':
                script.filePntrPos++;
                break;
            case 'v':
                script.filePntrPos--;
                break;
            case '+':
                program.arr[program.pntrPos]++;
                break;
            case '-':
                program.arr[program.pntrPos]--;
                break;
            case '.':
                if(script.isFileLoading){
                    script.file = realloc(script.file, (script.fileLength + 1) * sizeof(char));
                    script.file[script.fileLength] = program.arr[program.pntrPos];
                    script.fileLength++;
                }else if(script.fileMode){
                    fseek(script.openFile, script.filePntrPos, SEEK_SET);
                    size_t written_byte = fwrite(&program.arr[program.pntrPos], sizeof(char), 1, script.openFile);
                }else{
                    putchar(program.arr[program.pntrPos]);
                    //printf("%d ", arr[pntrPos]);
                }
                break;
            case ',':
                if(script.fileMode){
                    //char *buffer;
                    //buffer = (char*)malloc(1);
                    fseek(script.openFile, script.filePntrPos, SEEK_SET);
                    //size_t read_byte = fread(buffer, sizeof(char), 1, script.openFile);
                    //program.arr[program.pntrPos] = *buffer;
                    size_t read_byte = fread(&program.arr[program.pntrPos], 1, 1, script.openFile);
                    if(read_byte != 1){
                        perror("aaaa");
                    }
                    //putchar(buffer);
                }else{
                    program.arr[program.pntrPos] = getchar();
                }
                break;
            case '[':
                if (program.arr[program.pntrPos] == 0) {
                    int depth = 1;
                    while (depth > 0 && ++i < strlen(script.buffer)) {
                        if (cmd == '[') depth++;
                        else if (cmd == ']') depth--;
                    }
                } else {
                    script.loopStarts = realloc(script.loopStarts, (script.loops + 1) * sizeof(int));
                    script.loopStarts[script.loops++] = i;
                }
                break;
            case ']':
                if (program.arr[program.pntrPos] != 0 && script.loops > 0) {
                    i = script.loopStarts[script.loops - 1];
                } else if (script.loops > 0) {
                    script.loops--;
                    script.loopStarts = realloc(script.loopStarts, script.loops * sizeof(int));
                }
                break;
            case '{':
                if(!script.isFileLoading){
                    script.fileLength = 0;
                    script.file = (unsigned char*)malloc(sizeof(char));
                    script.isFileLoading = true;
                }else{
                    printf("\nError: Nested file loading in %s at character %d", script.filename, i);
                    return 1;
                }
                break;
            case '}':
                if(!script.fileMode){
                    BfScript nestedScript = EmptyScript();
                    nestedScript = LoadFile(nestedScript, script.file);
                    ExecuteScript(program, nestedScript);
                }else{
                    // Load file for read and write
                    //strcpy(script.openFileBuffer, nestedScript.buffer);
                    //printf("%s", script.file);
                    script.openFile = fopen(script.file, "rb+");
                    if(script.openFile == NULL){
                        perror("Error opening nested file");
                    }
                }
                free(script.file);
                script.isFileLoading = false;
                break;
            case '(':
                script.fileMode = true;
                break;
            case ')':
                script.fileMode = false;
                break;
            case ';':
                i = FindChar(script.buffer, '\n', i)-1;
                break;
            case '/':
                i = FindChar(script.buffer, '\\', i)-1;
                break;
        }
    }
    
    free(script.buffer);
    free(script.loopStarts);
    
    return 1;
}
