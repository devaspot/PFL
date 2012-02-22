#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

tree expr(int p, parser_ctx *c);
tree expression(parser_ctx *c) { return expr(-3, c); }
tree newnode(syntaxclass k) { tree p = (tree)malloc(sizeof(node)); p->tag=k; return p; }

void error(char *m, parser_ctx *c)
{ 
    printf("\n");
    printf("error:%s lineno=%i ch=<%c>(%i) sy=%i last=%s\n", m, c->lineno, c->ch, (int)(c->ch), (int)(c->sy), c->theword);
    printf("\n"); 
} 

void getch(parser_ctx *c)
{
    c->ch = fgetc(c->stream);
    if (c->ch == EOF) { c->ch = '.'; }
    else if (c->ch == '\n') { c->ch = ' '; c->lineno++; }
    else if (c->ch == '\t' || c->ch == '\r') { c->ch = ' '; }
}  

void insymbol(parser_ctx *c)
{
    char blank[] = "          ";
    int len=0;
    do { 
        while (c->ch==' ') getch(c);
        if (c->ch=='{')   
        {
            do {
                getch(c);
            } while (c->ch=='}' || c->ch == EOF);
            getch(c);
        }
    } while (c->ch == ' ' || c->ch == '{');
      
    if (c->ch == EOF) c->sy=eofsy;
    else if (c->ch >= 'a' && c->ch <= 'z' || c->ch >= 'A' && c->ch <= 'Z')                             
    { 
        strcpy(c->theword, blank); 
        while (c->ch >= 'a' && c->ch <= 'z' || c->ch >= 'A' && c->ch <= 'Z' || c->ch >= '0' && c->ch <= '9')
        {
            if (len<IDENTLEN) c->theword[len++] = c->ch; else c->theword[len]='\0';
            getch(c);
        }
        

             if (!strcmp(c->theword,"head      ")) c->sy=hdsy;  
        else if (!strcmp(c->theword,"tail      ")) c->sy=tlsy;
        else if (!strcmp(c->theword,"lambda    ")) c->sy=lambdasy;
        else if (!strcmp(c->theword,"if        ")) c->sy=ifsy;
        else if (!strcmp(c->theword,"then      ")) c->sy=thensy;
        else if (!strcmp(c->theword,"else      ")) c->sy=elsesy;
        else if (!strcmp(c->theword,"let       ")) c->sy=letsy;
        else if (!strcmp(c->theword,"in        ")) c->sy=insy;
        else if (!strcmp(c->theword,"rec       ")) c->sy=recsy;
        else if (!strcmp(c->theword,"or        ")) c->sy=orsy;
        else if (!strcmp(c->theword,"and       ")) c->sy=andsy;
        else if (!strcmp(c->theword,"not       ")) c->sy=notsy;
        else if (!strcmp(c->theword,"nil       ")) c->sy=nilsy;
        else if (!strcmp(c->theword,"null      ")) c->sy=nullsy;
        else if (!strcmp(c->theword,"true      ")) c->sy=truesy;
        else if (!strcmp(c->theword,"false     ")) c->sy=falsesy;
        else if (!strcmp(c->theword,"channel   ")) c->sy=chansy;
        else c->sy=word;
    } 
    else if (c->ch >= '0' && c->ch <= '9')
    { 
        c->theint=0;
        while (c->ch >= '0' && c->ch <= '9') 
        {
            c->theint=c->theint*10+(int)c->ch-(int)'0';
            getch(c);
        }
        c->sy=numeral;
    }
    else if (c->ch=='\'') 
    {
        getch(c);
        c->theword[1]=c->ch;
        getch(c);
        if (c->ch=='\'')                              
        {
            getch(c);
            c->sy=literal;
        }
        else error("char lit", c);
    }
    else if (c->ch > ' ' && c->ch < '0' || c->ch > '9' && c->ch < 'A' ||
             c->ch > 'Z' && c->ch < 'a' || c->ch > 'z' && c->ch <= '~')
    {
        switch (c->ch)
        {
            case '<': getch(c); if (c->ch=='=') { getch(c); c->sy=le; } else if (c->ch=='>') { getch(c); c->sy=ne; } else c->sy=lt; break;
            case '>': getch(c); if (c->ch=='=') { getch(c); c->sy=ge; } else c->sy=gt; break;
            case '(': getch(c); if (c->ch==')') { getch(c); c->sy=empty; } else c->sy=opensy; break;
            case ':': getch(c); if (c->ch==':') { getch(c); c->sy=conssy; } else c->sy=colon; break;
            case '-': getch(c); if (c->ch=='>') { getch(c); c->sy=sequencesy; } else c->sy=minus; break;
            case '|': getch(c); if (c->ch=='|') { getch(c); c->sy=parallelsy; } else c->sy=choicesy; break;
            case '+': c->sy=plus; getch(c); break;
            case '=': c->sy=eq; getch(c); break;
            case '*': c->sy=times; getch(c); break;
            case '/': c->sy=over; getch(c); break;
            case '.': c->sy=dot; getch(c); break;
            case ',': c->sy=comma; getch(c); break;
            case ')': c->sy=closesy; getch(c); break;
            case '[': c->sy=sqopen; getch(c); break;
            case ']': c->sy=sqclose; getch(c); break;
            case '"': c->sy=quote; getch(c); break;
            case '!': c->sy=outputsy; getch(c); break;
            case '?': c->sy=inputsy; getch(c); break;
         }
    } else 
         error("bad symbol", c);
}  

void check(symbol s, char *m, parser_ctx *c) { if (c->sy==s) insymbol(c); else error(m, c); }
int syis(symbol s, parser_ctx *c) { if (c->sy==s) { insymbol(c); return 1; } else return 0; }

tree param(parser_ctx *c)
{
    tree p;
    if (c->sy==word)
    {
        p=newnode(ident);
        strcpy(p->id,c->theword); 
    }
    else if (c->sy==empty)
         p=newnode(emptycon);
    else error("f param", c);
    insymbol(c);
    return p;
}

tree cons(symbol isrec, tree h,tree t)
{
    tree p;
    p=newnode(declist);
    p->declist.recursive=isrec;
    p->declist.head=h;
    p->declist.tail=t;
    return p;
}

tree pdec(parser_ctx *c)  
{
    tree d;
    if (c->sy==word) 
    {
        d=newnode(decln);
        strcpy(d->decln.name,c->theword); 
        insymbol(c);
        check(eq,"= expected", c);
        d->decln.value = expression(c);
    }                  
    else error("dec, no id", c);
    return d;
}

tree pdeclist(symbol isrec, parser_ctx *c) 
{
    tree d=pdec(c);
    if (syis(comma, c)) d=cons(isrec,d,pdeclist(isrec, c)); else d=cons(isrec,d,0);
    return d;
}

tree pdecs(parser_ctx *c) 
{
    tree d=newnode(block);
    symbol isrec=syis(recsy, c) ? recsy : unknown;    
    d->block.decs=pdeclist(isrec, c);
    check(insy, "in expected", c);
    d->block.expr = expression(c);
    return d;
} 

tree expr(int priority, parser_ctx *c)
{
    tree e = NULL;
    tree a = NULL;
    if (priority < 7) 
    {
        e=expr(priority+1, c);
        if ((((int64)1<<c->sy) & c->binoprs & c->rightassoc) && (c->oprpriority[c->sy]==priority)) 
        {
            a=e;
            e=newnode(binexp);
            e->expression.op = c->sy;
            insymbol(c);
            e->expression.left = a;
            e->expression.right = expr(priority, c);
        }
        else
        while ((((int64)1<<c->sy) & c->binoprs & ~c->rightassoc) && (c->oprpriority[c->sy]==priority))
        {   
            a=e;
            e=newnode(binexp);
            e->expression.op = c->sy;
            insymbol(c);
            e->expression.left = a;
            e->expression.right = expr(priority+1, c);
        }
    }
    else if (priority == 7)  
    {
        e=expr(priority+1, c);
        while (((int64)1<<c->sy) & c->startsexp & ~c->binoprs)  
        {
            a=e;
            e=newnode(application);
            e->application.func=a;
            e->application.parm = expr(priority+1, c);
        }
    }
    else if (((int64)1<<c->sy) & c->unoprs) 
    {
        e=newnode(unexp);
        e->expression.op = c->sy;
        insymbol(c);
        e->expression.left = expr(priority, c);
        e->expression.right = 0;
    }
    else if (((int64)1<<c->sy) & c->startsexp)
    switch (c->sy) {
        case word:     { e=newnode(ident); strcpy(e->id,c->theword); insymbol(c); } break;
        case numeral:  e=newnode(intcon); e->n=c->theint; insymbol(c); break;
        case literal:  e=newnode(charcon); e->ch=c->theword[1]; insymbol(c); break;
        case empty:    insymbol(c); e=newnode(emptycon); break;
        case nilsy:    insymbol(c); e=newnode(nilcon); break;
        case chansy:   insymbol(c); e=newnode(newchan); break;
        case truesy:   e=newnode(boolcon); e->b=1; insymbol(c); break;
        case falsesy:  e=newnode(boolcon); e->b=0; insymbol(c); break;
        case opensy:   insymbol(c);
                       e = expression(c);
                       check(closesy,") expected", c);
                       break;
        case letsy:    insymbol(c);
                       e=pdecs(c);
                       break;
        case ifsy:     insymbol(c);
                       e=newnode(ifexp);
                       e->ifexp.e1=expression(c);
                       check(thensy,"no then", c);
                       e->ifexp.e2=expression(c);
                       check(elsesy,"no else", c);
                       e->ifexp.e3=expression(c);
                       break;
        case lambdasy: insymbol(c);
                       e=newnode(lambdaexp);
                       e->lambda.parm=param(c);
                       check(dot,". expected", c);
                       e->lambda.body=expression(c);
                       break;
   }  
   else error("bad operand", c);

   return e;
}

void parser_init(parser_ctx *context)
{
    int sym;

    for(sym=word;sym<=eofsy; sym++) context->oprpriority[sym] = 0;

    context->startsexp  = (int64)1 << opensy | (int64)1 << letsy | (int64)1 << ifsy | 
                          (int64)1 << lambdasy | (int64)1 << chansy | context->unoprs | 0x7F;
    context->rightassoc = (int64)1 << conssy | (int64)1 << sequencesy;
    context->unoprs     = (int64)1 << minus | (int64)1 << hdsy | (int64)1 << tlsy | (int64)1 << nullsy | (int64)1 << notsy;
    context->binoprs    = (int64)1 << sequencesy | (int64)1 << parallelsy | (int64)1 << choicesy | (int64)1 << inputsy |
                          (int64)1 << outputsy | (int64)1 << conssy | (int64)1 << orsy | (int64)1 << andsy |
                          (int64)1 << eq | (int64)1 << ne | (int64)1 << lt | (int64)1 << le | (int64)1 << gt |
                          (int64)1 << ge | (int64)1 << plus | (int64)1 << minus | (int64)1 << times | (int64)1 << over; 

    context->oprpriority[parallelsy] = -3; 
    context->oprpriority[choicesy]   = -2;
    context->oprpriority[sequencesy] = -1;
    context->oprpriority[inputsy]    =  0;
    context->oprpriority[outputsy]   =  0;
    context->oprpriority[conssy]     =  1;
    context->oprpriority[orsy]       =  2;
    context->oprpriority[andsy]      =  3;

    for (sym=eq; sym<=ge; sym++) context->oprpriority[sym] = 4;

    context->oprpriority[plus]  = 5;
    context->oprpriority[minus] = 5;
    context->oprpriority[times] = 6;
    context->oprpriority[over]  = 6;

    context->lineno = 1;
    context->ch = ' ';
    context->theint = 0xc0fefa;
}

tree parse(parser_ctx *context)
{
    insymbol(context);
    return expression(context); 
}
