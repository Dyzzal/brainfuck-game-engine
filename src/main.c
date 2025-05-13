/*
 * TODO:
 *  -File functions
 *      -Add file importing to BF (load file name in {} by outputting chars, empty {} close file) 
 *      -File I/O by using ^ and v to change the current byte of the file and using , to read byte to register and . to write byte from register 
 *  -Temporary register
 */

#include "bf.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  BfProgram program;
  BfScript script = EmptyScript();

  script = LoadFile(script, argv[1]);

  ExecuteScript(program, script, 0);

  return 0;
}
