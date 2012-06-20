
#include "parser.h"

static void printid(alfa id)
{
    int i=0;
    while (i<=IDENTLEN) if (id[i]==' ') break; else { printf("%c", id[i++]); }
}

static void printsy(symbol sy)
{
    switch (sy) {
        case parallelsy: printf("||"); break;
        case choicesy:   printf("|"); break;
        case sequencesy: printf("->"); break;
        case inputsy:    printf("?"); break;
        case outputsy:   printf("!"); break;
        case conssy:     printf(":"); break;
        case orsy:       printf("or"); break;
        case andsy:	 printf("and"); break;
        case eq:   	 printf("="); break;
        case ne:   	 printf("<>"); break;   
        case le:   	 printf("<="); break;
        case ge:   	 printf(">="); break;    
        case gt: 	 printf(">"); break;    
        case lt:	 printf("<"); break;
        case plus:	 printf("+"); break;     
        case minus: 	 printf("-"); break;
        case times: 	 printf("*"); break;     
        case over: 	 printf("/"); break;
        case hdsy:	 printf("head"); break;   
        case tlsy: 	 printf("tail"); break;
        case nullsy:	 printf("null"); break; 
        case notsy:      printf("not"); break;
    }
}

#define TAB 4
void print(int deep, tree t)
{
    int i;
    if (t!=0) 
    {
        switch (t->tag) {
            case ident:       printid(t->id); break;
            case intcon:      printf("%i", t->n); break;
            case boolcon:     printf("%i", t->b); break;
            case charcon:     printf("\'%c' ", t->ch); break;
            case emptycon:    printf("()"); break;
            case nilcon:      printf("nil"); break;
            case newchan:     printf("chan"); break;
            case lambdaexp:   printf("lambda "); print(deep,t->lambda.parm); printf(". "); print(deep,t->lambda.body); break;
            case application: print(deep,t->application.func); printf(" "); print(deep,t->application.parm); break;
            case unexp:       printsy(t->expression.op); printf(" "); print(deep,t->expression.left); break;
            case binexp:      print(deep,t->expression.left); printf(" "); printsy(t->expression.op); printf(" "); print(deep,t->expression.right); break;
            case ifexp:       printf("if "); print(deep,t->ifexp.e1); printf(" then "); print(deep,t->ifexp.e2); printf(" else "); print(deep,t->ifexp.e3); break;
            case declist:     print(deep,t->declist.head); if (t->declist.tail != 0) printf(",\n"); 
                              print(deep,t->declist.tail); break;
            case decln:       for (i=0;i<deep*TAB;i++) printf(" "); printid(t->decln.name); printf(" = "); print(deep,t->decln.value); break;
            case block:       printf("let "); 
                              if (t->block.decs->declist.recursive == recsy) printf("rec\n"); else printf("\n");
                              print(deep+1,t->block.decs); printf("\n");
                              for (i=0;i<deep*TAB;i++) printf(" "); printf(" in "); print(deep+1,t->block.expr);
                              break;
        }
    }
}
