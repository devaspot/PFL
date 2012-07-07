// ML execution

#ifndef EXEC_H
#define EXEC_H

#include "parser.h"

#define MAXFUN 1024

// bytecodes
typedef enum { intval, boolval, charval, emptyval, listval,
               nilval, funcval, deferval,
               inprocessval, outprocessval,           /* c?x->..., c!e->... */
               choiceprocessval, paraprocessval,      /* ...|...,  ...||... */
               stopprocessval, channelval, last_valueclass, procid} valueclass;

// atom table
typedef struct binding* env;
typedef struct valnode* value;
typedef struct binding { alfa id; value v; env next; } binding;

// execution
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

typedef struct exec_ctx* exec_context;
typedef struct exec_ctx {
   int evals, envcells, conscells; // statistics 
   alfa lastid;                    // debugging
   int64 processvalues;
   int channelcntr;
   value processes, outputchan, outputproc, inputchan, inputproc;
   tree outputmsg, inputmsg, outputcont, inputcont;
   env sysenv;
   int n;
   exec_context next;
} exec_ctx;

exec_ctx proctable[MAXFUN];

void execute(tree prog, exec_ctx *c);

#endif
