SRC_DIR ?= src/
make:
	#mkdir bin
	gcc $(SRC_DIR)bf.c $(SRC_DIR)main.c -o bin/bf 
