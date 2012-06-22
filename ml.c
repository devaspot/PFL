//
//               Robin Milner's Meta Language 
//
// Implemented:  AE   Applicative Engine (lambda calculus)
//               CCS  Calculus of Communicating Systems (process calculus)
//               LE   Lazy Evaluations
//
// Future Tasks: TI   Type Inference
//               DT   Dependent Types
//               LLVM Compilation
//
// Homepage:     http://github.com/synrc/research-ml
// Authors:      Maxim Sokhatsky <maxim@synrc.com>
// Legal Info:   2011-2012 (c) Synrc Research Center
//               Distributed under MIT License
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "exec.h"

int main(int argc, char *argv[])
{
    int i;
    FILE *stream;
    tree program;
    parser_ctx parser;
    struct exec_ctx *root = (exec_ctx*)malloc(sizeof(exec_ctx));
    proctable[0] = *root; //(exec_ctx*)malloc(sizeof(exec_ctx));

    exec_ctx *exec = &proctable[0];
//    exec->next = 0;

    if (argc < 2) stream = stdin; else stream = fopen(argv[1],"r+");
    if (stream==0) { printf("Fail. Cannot open input stream."); exit(1); }
    parser.stream = stream;
    printf("Synrc ML Jun 2012\n");
    parser_init(&parser);
    program = parse(&parser);
    print(0, program);
    printf("\n");
    clock_t start = clock();
    execute(program, exec);
    printf("execution time: %2.2fs\n", ( (double)clock() - start ) / CLOCKS_PER_SEC);
    return 0;
}
