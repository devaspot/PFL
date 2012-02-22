SRC=ml.c parse.c print.c exec.c 

all: $(SRC)
	gcc $(SRC) -o ml.exe -fno-builtin-strcpy
