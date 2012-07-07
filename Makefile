SRC=runtime/ml.c runtime/parse.c runtime/print.c runtime/exec.c 

all: $(SRC)
	gcc $(SRC) -o ml -I include -fno-builtin-strcpy
