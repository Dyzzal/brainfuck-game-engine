SRC_DIR ?= src/

# Platform: WINDOWS, LINUX
PLATFORM ?= LINUX
make:
	#mkdir bin
	gcc $(SRC_DIR)bf.c $(SRC_DIR)main.c -o bin/bf 

