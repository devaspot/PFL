#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "exec.h"

int main(int argc, char *argv[])
{
    FILE *stream;
    tree program;
    parser_ctx parser;
    exec_ctx exec;
    if (argc < 2) stream = stdin; else stream = fopen(argv[1],"r+");
    if (stream==0) { printf("Fail. Cannot open input stream."); exit(1); }
    parser.stream = stream;
    printf("Synrc ML Feb 2012\n");
    parser_init(&parser);
    program = parse(&parser);
    print(0, program);
    printf("\n");
    execute(program, &exec);
    return 0;
}
