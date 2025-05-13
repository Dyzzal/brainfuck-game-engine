#include "bf.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#define EXECUTE(NEST) ExecuteScript(NEST.program, NEST.script, NEST.sleep)

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

  return -1;
}

BfScript EmptyScript() {
  BfScript script;
  script.bfFile = 0;
  script.buffer = malloc(sizeof(char));
  script.file = malloc(sizeof(char));
  script.fileMode = false;
  script.fileLength = 0;
  script.isFileLoading = false;
  script.filePntrPos = 0;
  script.loopStarts = malloc(sizeof(int));
  script.loops = 0;
  return script;
}

BfScript LoadFile(BfScript script, const char *filename) {
  // Load file
  script.bfFile = fopen(filename, "r");
  if (script.bfFile == NULL) {
    perror("Error loading file");
  }
  script.filename = (char *)filename;

  // Get file size for buffer
  fseek(script.bfFile, 0, SEEK_END);
  long file_size = ftell(script.bfFile);
  rewind(script.bfFile);

  // Allocate memory to buffer
  script.buffer =
      (unsigned char *)malloc(sizeof(unsigned char) * (file_size + 1));
  if (script.buffer == NULL) {
    perror("Error allocating memory");
    fclose(script.bfFile);
  }

  // Read file to buffer
  size_t bytes_read = fread(script.buffer, 1, file_size, script.bfFile);
  if (bytes_read != file_size) {
    perror("Error reading file");
    fclose(script.bfFile);
    free(script.buffer);
  }

  // Add null-terminator to the end of the buffer
  script.buffer[file_size] = '\0';

  return script;
}

#ifdef _WIN32
DWORD WINAPI ExecuteNestedScript(LPVOID arg) {
#else
void *ExecuteNestedScript(void *arg) {
#endif
  NestedScript *script = (NestedScript *)arg;
  ExecuteScript(script->program, script->script, script->sleep);
  free(script);
#ifdef _WIN32
  return 0;
#else
  return NULL;
#endif
}

int ExecuteScript(BfProgram program, BfScript script, float sleep) {
  script.loopStarts = malloc(sizeof(int));

  for (int i = 0; i < strlen(script.buffer); ++i) {
    unsigned char cmd = script.buffer[i];
    switch (cmd) {
    case '>':
      script.pntrPos++;
      if (script.pntrPos > sizeof(program.arr)) {
       script.pntrPos -= sizeof(program.arr);
      }
      break;
    case '<':
      script.pntrPos--;
      if (script.pntrPos < 0) {
        script.pntrPos += sizeof(program.arr);
      }
      break;
    case '^':
      script.filePntrPos++;
      break;
    case 'v':
      script.filePntrPos--;
      break;
    case '+':
      program.arr[script.pntrPos]++;
      break;
    case '-':
      program.arr[script.pntrPos]--;
      break;
    case '.':
      if (script.isFileLoading) {
        script.file =
            realloc(script.file, (script.fileLength + 1) * sizeof(char));
        script.file[script.fileLength] = program.arr[script.pntrPos];
        script.fileLength++;
      } else if (script.fileMode) {
        fseek(script.openFile, script.filePntrPos, SEEK_SET);
        size_t written_byte = fwrite(&program.arr[script.pntrPos], sizeof(char),
                                     1, script.openFile);
      } else {
        putchar(program.arr[script.pntrPos]);
        // printf("%d ", program.arr[script.pntrPos]);
        fflush(stdout);
      }
      break;
    case ',':
      if (script.fileMode) {
        if (script.openFile == NULL) {
          perror("File does not exist");
        }
        fseek(script.openFile, script.filePntrPos, SEEK_SET);
        size_t read_byte =
            fread(&program.arr[script.pntrPos], 1, 1, script.openFile);
      } else {
        program.arr[script.pntrPos] = getchar();
      }
      break;
    case '[':
      if (program.arr[script.pntrPos] == 0) {
        int depth = 1;
        while (depth > 0 && ++i < strlen(script.buffer)) {
          if (script.buffer[i] == '[')
            depth++;
          else if (script.buffer[i] == ']')
            depth--;
        }
      } else {
        script.loopStarts =
            realloc(script.loopStarts, (script.loops + 1) * sizeof(int));
        script.loopStarts[script.loops++] = i;
      }
      break;
    case ']':
      if (program.arr[script.pntrPos] != 0 && script.loops > 0) {
        i = script.loopStarts[script.loops - 1];
      } else if (script.loops > 0) {
        script.loops--;
        script.loopStarts =
            realloc(script.loopStarts, script.loops * sizeof(int));
      }
      break;
    case '{':
      if (!script.isFileLoading) {
        script.fileLength = 0;
        script.file = (unsigned char *)malloc(sizeof(char));
        script.isFileLoading = true;
      } else {
        printf("\nError: Nested file loading in %s at character %d",
               script.filename, i);
        return 1;
      }
      break;
    case '}':
      if (!script.fileMode) {
        NestedScript *nest = malloc(sizeof(NestedScript));
        *nest = (NestedScript){.program = program,
                               .script = LoadFile(EmptyScript(), script.file),
                               .sleep = sleep};
        nest->script.pntrPos = script.pntrPos;

#ifdef _WIN32
        HANDLE thread =
            CreateThread(NULL, 0, ExecuteNestedScript, nest, 0, NULL);
        if (thread == NULL) {
          perror("CreateThread failed");
          free(nest);
        } else {
          CloseHandle(thread);
        }
#else
        pthread_t thread;
        if (pthread_create(&thread, NULL, ExecuteNestedScript, nest) != 0) {
          perror("pthread_create failed");
          free(nest);
        } else {
          pthread_join(thread, NULL);
        }
#endif
      } else {
        // Load file for read and write
        script.openFile = fopen(script.file, "rb+");
        if (script.openFile == NULL && errno == ENOENT) {
          script.openFile = fopen(script.file, "wb+");
        }
        if (script.openFile == NULL) {
          perror("Error opening or creating file");
          return 1;
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
      if (script.openFile != NULL) {
        fflush(script.openFile);
        fclose(script.openFile);
        script.openFile = NULL;
      }
      break;
    case ';':
      i = FindChar(script.buffer, '\n', i) - 1;
      break;
    case '/':
      i = FindChar(script.buffer, '\\', i) - 1;
      break;
    }
    usleep(sleep * 1000);
  }

  free(script.buffer);
  free(script.loopStarts);
  return 1;
}
