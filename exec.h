#ifndef EXEC_H
#define EXEC_H

#include "parser.h"

typedef struct binding* env;
typedef struct valnode* value;
typedef struct binding { alfa id; value v; env next; } binding;
typedef enum {intval=0, boolval=1, charval=2, emptyval=3, listval=4,
              nilval=5, funcval=6, deferval=7,
              inprocessval=8, outprocessval=9,           /*c?x->..., c!e->...*/
              choiceprocessval=10, paraprocessval=11,      /*...|...,  ...||...*/
              stopprocessval=12, channelval=13, last_valueclass=14} valueclass;
typedef int64 values;
typedef struct valnode {
    valueclass tag;
    union {
        struct { value p1, p2; } proc; // .|., .||.
        struct { value chan; tree msg, cont; env pr; } ioproc; // c?x->..., c!e->...
        int n; 
        int b; 
        char ch; 
        struct { value hd, tl; } list;
        struct { tree e; env r; } func;
    };
} valnode;
typedef struct exec_ctx {
   int evals, envcells, conscells; // statistics 
   alfa LastId;                 // debugging
   int64 processvalues;
   int ChannelCntr;
   value Processes, OutputChan, OutputProc, InputChan, InputProc;
   tree OutputMsg, InputMsg, OutputCont, InputCont;
   env SysEnv;
   int n;
} exec_ctx;

void execute(tree prog, exec_ctx *c);

#endif
