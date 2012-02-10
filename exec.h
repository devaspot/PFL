#ifndef EXEC_H
#define EXEC_H

#include "parser.h"

typedef struct binding* env;
typedef struct valnode* value;
typedef struct binding { alfa id; value v; env next; } binding;
typedef enum {intval, boolval, charval, emptyval, listval, nilval, funcval, deferval,
              inprocessval, outprocessval,           /*c?x->..., c!e->...*/
              choiceprocessval, paraprocessval,      /*...|...,  ...||...*/
              stopprocessval, channelval, last_valueclass} valueclass;
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
