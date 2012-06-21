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

#include "exec.h"

int main(int argc, char *argv[])
{
    int i;
    FILE *stream;
    tree program;
    parser_ctx parser;
    exec_ctx *exec = &proctable[0]; inc_proc(1);
    if (argc < 2) stream = stdin; else stream = fopen(argv[1],"r+");
    if (stream==0) { printf("Fail. Cannot open input stream."); exit(1); }
    parser.stream = stream;
    printf("Synrc ML Jun 2012\n");
    parser_init(&parser);
    program = parse(&parser);
//    print(0, program);
    printf("\n");
    execute(program, exec);
    return 0;
}
