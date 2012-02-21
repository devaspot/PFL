#include <stdlib.h>
#include "exec.h"

void error(char *m, alfa lastid) { printf("\nError: %s LastId=%s\n", m, lastid); }
value checkval(char *m, values vs, value v, alfa lastid) { if (!(v->tag & vs)) error(m, lastid); return v; }
value mkvalue(valueclass t) { value p=(value)malloc(sizeof(valnode)); p->tag=t; return p; }
value mkint(int nn) { value p=(value)malloc(sizeof(valnode)); p->tag=intval; p->n=nn; return p; }
value mkbool(int bb) { value p=(value)malloc(sizeof(valnode)); p->tag=boolval; p->b=bb; return p; }
value mkchar(char cc) { value p=(value)malloc(sizeof(valnode)); p->tag=charval; p->ch=cc; return p; }
value mkfunc(tree code, env rho) { value p=(value)malloc(sizeof(valnode)); p->tag=funcval; p->func.e=code; p->func.r=rho; return p; }
value defer(tree x, env rho) { value p=(value)malloc(sizeof(valnode)); p->tag=deferval; p->func.e=x; p->func.r=rho; return p; }
value cons(value h, value t, int *conscells) { value p=(value)malloc(sizeof(valnode)); p->tag=listval; p->list.hd=h; p->list.tl=t; *conscells++; return p; }

value mkprocess1(valueclass t, value pa, value pb, alfa lastid, int64 processvalues)
{
    value p=(value)malloc(sizeof(valnode));
    p->tag=t;  //choice or para, | or ||, NB. pA, pB in WHNF
    p->proc.p1=checkval("bad left proc", processvalues, pa, lastid);
    p->proc.p2=checkval("bad right proc", processvalues, pb, lastid);
    return p;
}

value mkprocess2(valueclass t, value chan, tree m, tree c, env rho)
{
    value p=(value)malloc(sizeof(valnode));
    p->tag=t; //choice or para, | or ||, NB. pA, pB in WHNF
    p->ioproc.chan=chan;
    p->ioproc.msg=m;
    p->ioproc.cont=c;
    p->ioproc.pr=rho;
    return p;
}

value mkchannel(int* channelcntr, int* n)
{
    value p=(value)malloc(sizeof(valnode));
    p->tag=channelval;  //choice or para, | or ||, NB. pA, pB in WHNF
    *channelcntr++;
    *n=*channelcntr;
    return p;
}

env bind(alfa x, value val, env r, int* envcells) // :Ide -> Value -> Env -> Env 
{
    env p=(env)malloc(sizeof(binding));
    *envcells++;
    strcpy(p->id,x);
    p->v=val;
    p->next=r;
    return p;
}

value applyenv(env r, alfa x, alfa lastid, int* channelcntr, int* n, int* envcells, int64 processvalues, int* conscells, int* evals)      /* :Env -> Ide -> Value */
{ 
    strcpy(lastid,x);
    if (r==0) error("undec id", lastid);
    else if (!strcmp(r->id,x)) 
    {
        force(r->v, lastid, channelcntr, n, envcells, processvalues, conscells, evals ); //only called from eval
        return r->v;
    }
    else return applyenv( r->next, x, lastid, channelcntr, n, envcells, processvalues, conscells, evals );
}

env d2(tree decs, env local, env global, int* envcells)
{ 
    if (decs==0) return global;
    else return bind(decs->declist.head->decln.name, defer(decs->declist.head->decln.value,local), d2(decs->declist.tail, local, global, envcells), envcells);
}

env d(tree decs, env rho, int* envcells) // D :Decs -> Env -> Env
{
    env localrho;

    if (decs==0) return rho;
    else
    {
        if (decs->declist.recursive) 
        {
            localrho=bind("dummy", 0, rho, envcells);
            localrho->next=d2(decs, localrho, rho, envcells);
            return localrho;
            }
            else return d2(decs, rho, rho, envcells);
    }
}

value apply(value fn, value ap, alfa lastid, int* envcells, int* channelcntr, int* n, int64 processvalues, int* conscells, int* evals) // apply a function fn to param ap
{
    if (fn->func.e->lambda.parm->tag == emptycon)   // (L().e)ap 
    {
        force(ap, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        if (ap->tag == emptyval) 
//eval(tree x, env rho, alfa lastid, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals)
            return eval(fn->func.e->lambda.body, fn->func.r, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        else 
        {
            error("L().e exp", lastid);
            return 0;
        }
    }
    else return eval(fn->func.e->lambda.body, bind(fn->func.e->lambda.parm->id, ap, fn->func.r, envcells), lastid, channelcntr, n, envcells, processvalues, conscells, evals);
}
/*
void o(symbol opr, value v1, value v2 , alfa lastid, int64 processvalues, int* conscells) // O :Value^2 -> Value 
{
    int abs1, abs2, intans;
    int boolans;
     void o_result;
      switch (opr) {
      case parallelsy: //...||...
           o_result=mkprocess1(paraprocessval,  v1,v2, lastid, processvalues); break;
      case choicesy:   //...|.... 
           o_result=mkprocess1(choiceprocessval,v1,v2, lastid, processvalues); break;

      case eq: case ne: case lt: case le: case gt: case ge:
         { if (!equivalent(intersect(intersect(setof(v1->tag, eos), setof(v2->tag, eos)), setof(intval, boolval, charval, eos)), setof(eos))) 
               switch (v1->tag) {
               case intval:  { abs1=v1->n;       abs2=v2->n; } break;
               case boolval: { abs1=ord(v1->b);  abs2=ord(v2->b);  } break;
               case charval: { abs1=ord(v1->ch); abs2=ord(v2->ch); } break;
               }
               else error("rel ops   ", lastid);
               switch (opr) {
               case eq: boolans=abs1== abs2; break;   case ne: boolans=abs1!=abs2; break;
               case le: boolans=abs1<=abs2; break;   case lt: boolans=abs1< abs2; break;
               case ge: boolans=abs1>=abs2; break;   case gt: boolans=abs1> abs2;
               break;
               }
               o_result=mkbool(boolans);
         }
         break;        
      case plus: case minus: case times: case over:
         { if (equivalent(setof(v1->tag, v2->tag, eos), setof(intval, eos))) 
               switch (opr) {
               case plus:  intans=v1->n +   v2->n; break;
               case minus: intans=v1->n -   v2->n; break;
               case times: intans=v1->n *   v2->n; break;
               case over:  intans=v1->n / v2->n;
               break;
               }
               else error("arith opr ", lastid);
               o_result=mkint(intans);
         }
         break;
      case andsy: case orsy:
         { if (equivalent(setof(v1->tag, v2->tag, eos), setof(boolval, eos))) 
               switch (opr) {
               case andsy: boolans=v1->b & v2->b; break;
               case orsy:  boolans=v1->b |  v2->b;
               break;
               }
               else error("bool opr  ", lastid);
               o_result=mkbool(boolans);
         }
         break;
      case conssy: // deferred params 
          o_result=cons(v1, v2, conscells);
      break;
      }
      return o_result;
}

static void u( symbol opr, value v , void* lastid, integer* channelcntr, integer* n, integer* envcells, set* processvalues, integer* conscells, integer* evals)    
// U :Value -> Value 
//PRE: v^.tag <> deferval
{ void u_result;
      switch (opr) {
      case minus: if (v->tag==intval)  u_result=mkint(-v->n);
             else error("- non int ", lastid);
             break;
      case notsy: if (v->tag==boolval)  u_result=mkbool(~ v->b);
             else error("not ~bool ", lastid);
             break;
      case hdsy:  if (v->tag==listval) 
                { force(v->hd, lastid, channelcntr, n, envcells, processvalues, conscells, evals); u_result=v->hd; }
             else error("hd ~list  ", lastid);
             break;
      case tlsy:  if (v->tag==listval) 
                { force(v->tl, lastid, channelcntr, n, envcells, processvalues, conscells, evals); u_result=v->tl; }
             else error("tl ~list  ", lastid);
             break;
      case nullsy:if (v->tag==listval)      u_result=mkbool(false);
             else if (v->tag==nilval)  u_result=mkbool(true);
             else error("null ~list", lastid);
             break;
      }
      return u_result;
}
*/

value eval(tree x, env rho, alfa lastid, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals)
{
    value a;
    return a;
}

/*

 eval :Exp -> Env -> Value  Note: evaluates an Expression and returns a Value*
 POST: result tag is not deferval, weak head normal form
{
       value func, switch_, chnl; valueclass proctag;
      void eval_result;
      switch (x->tag) {
         case ident:     eval_result=applyenv(rho, x->id, lastid, channelcntr, n, envcells, processvalues, conscells, evals); break;
         case intcon:    eval_result=mkint(x->n); break;
         case boolcon:   eval_result=mkbool(x->b); break;
         case charcon:   eval_result=mkchar(x->ch); break;
         case nilcon:    eval_result=mkvalue(nilval); break;
         case emptycon:  eval_result=mkvalue(emptyval); break;
         case newchan:   eval_result=mkchannel(channelcntr, n); break;
         case lambdaexp: eval_result=mkfunc(x, rho); break;
         case application:
            { func = eval(x->fun, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                  if (func->tag==funcval) 
                     eval_result=apply(func, defer(x->aparam, rho), lastid, envcells, channelcntr, n, processvalues, conscells, evals);
                  else error("apply ~fn ", lastid);
            }
            break;
         case unexp:   eval_result=u(x->unopr, eval(x->unarg, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals), lastid, channelcntr, n, envcells, processvalues, conscells, evals); break;
         case binexp:  if (x->binopr==sequencesy)                   //->
                     if (x->left->tag == binexp) 
                     {
                        if (x->left->binopr == inputsy)         //...?...->
                        { proctag=inprocessval;
                              if (x->left->right->tag != ident) 
                                 error("...?~var  ", lastid);
                        }
                        else if (x->left->binopr == outputsy)   //...!...->
                           proctag=outprocessval;
                        else error("~\?/! ->...", lastid);
                        chnl=eval(x->left->left,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                        if (chnl->tag != channelval)  error("chan xpctd", lastid);
                        eval_result=mkprocess2(proctag,
                                         chnl,x->left->right,x->right,rho);
                     }
                     else error("~IO -> ...", lastid);

                  else if (inset(x->binopr, setof(inputsy,outputsy, eos))) 
                     //An action is part of a process, not a Value, ?yet? 
                     // NB. an input action needs a Cont to take in-value  
                     error("eval ? | !", lastid);

                  else if (x->binopr==conssy) // cons should not eval ... 
                       eval_result=o(x->binopr, defer(x->left,rho),
                                          defer(x->right,rho), lastid, processvalues, conscells);
                  else eval_result=o(x->binopr, eval(x->left,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals),   
                                //others strict
                                          eval(x->right,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals), lastid, processvalues, conscells);
                                          break;
         case ifexp:
            { switch_=eval(x->e1, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                  if (switch_->tag==boolval) 
                     if (switch_->b)  eval_result=eval(x->e2, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                                  else eval_result=eval(x->e3, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                  else error("if ~bool  ", lastid);
            }
            break;
         case block:   eval_result=eval( x->exp, d(x->decs, rho, envcells), lastid, channelcntr, n, envcells, processvalues, conscells, evals);
         break;
      }   
        *evals = *evals + 1; // statistics 
      return eval_result;
}

static void force( value v , void* lastid, integer* channelcntr, integer* n, integer* envcells, set* processvalues, integer* conscells, integer* evals)
{
       value fv;
      if (v->tag==deferval) 
         { fv = eval( v->e, v->r, lastid, channelcntr, n, envcells, processvalues, conscells, evals ); v = fv; }
}

static void showvalue( value v , integer* n, void* lastid, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals)
{ { void* with = v;  
      switch (tag) {
      case intval:  cwrite("%1i",  *n ); break;
      case boolval: cwrite("%?",  b ); break;
      case charval: cwrite("%?",  ch ); break;
      case emptyval:cwrite("{}" ); break;
      case nilval:  cwrite("nil"); break;
      case listval: { cwrite("{"); showvalue(hd, n, lastid, channelcntr, envcells, processvalues, conscells, evals); cwrite(","); showvalue(tl, n, lastid, channelcntr, envcells, processvalues, conscells, evals); cwrite("}"); } break;
      case funcval: cwrite("function"); break;
      case inprocessval: case outprocessval:
      case choiceprocessval: case paraprocessval: cwrite("process");
      break;
      case stopprocessval: cwrite("stop"); break;
      case channelval: cwrite("chan%1i", *n); break;
      case deferval:{ force(v, lastid, channelcntr, n, envcells, processvalues, conscells, evals); showvalue(v, n, lastid, channelcntr, envcells, processvalues, conscells, evals); } break; // evaluation is o/p driven 
      }}
}

static boolean interact(value* processes, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals);
static boolean findip(value* ip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* processes);
static boolean traverseip(value subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* ip, void* processes);
static boolean findop(value* op, void* subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* ip);
static boolean traverseop(value subop, void* subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* op, void* ip);


static boolean complementary(void* subip, void* subop, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* op, void* ip)
{
       integer chan; value val;
      //?can IP and OP interact?
      boolean complementary_result;
      complementary_result = false; //unless ...
      chan   = (*subip)->chnl->n;
      if (chan == (*subop)->chnl->n)  //exchange msg
      { complementary_result=true;
            if (chan==2)//input 
                //val:= ...
             error("not impl  ", lastid);
            else val=defer((*subop)->msg,(*subop)->pr);


            if (chan==1)  { 
               cwrite("\n"); cwrite(" output "); showvalue(val, n, lastid, channelcntr, envcells, processvalues, conscells, evals); 
            } else {
               cwrite(" ch%1i ", chan); showvalue(val, n, lastid, channelcntr, envcells, processvalues, conscells, evals);
            }

            *op=eval((*subop)->cont,(*subop)->pr, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
            *ip=eval((*subip)->cont,bind((*subip)->msg->id,val,(*subip)->pr, envcells), lastid, channelcntr, n, envcells, processvalues, conscells, evals);
}   return complementary_result;
}
                                                  //OP - Output Process
static boolean traverseop(value subop, void* subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* op, void* ip)

{ 
      //find an output process
      boolean traverseop_result;
      switch (subop->tag) {
      case inprocessval:     traverseop_result=false; break;
      case outprocessval:    traverseop_result=//?
      complementary(subip, &subop, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip); break;
      case choiceprocessval: if (traverseop(subop->p1, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip)) 
                           traverseop_result=true;
                        else traverseop_result=traverseop(subop->p2, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip);
                        break;
      case stopprocessval:   traverseop_result=false; break; //can't interact!
      case paraprocessval:   error("|| under |", lastid);
      break;
}   return traverseop_result;
}


static boolean findop(value* op, void* subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* ip)

{ boolean findop_result;
      if ((*op)->tag==paraprocessval)   //p1||p2
           if (findop(&(*op)->p1, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip))  findop_result=true;
           else findop_result=findop(&(*op)->p2, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip);
      else findop_result=traverseop(*op, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip); //p1|p2, a->p, stop
      return findop_result;
}
                                                  //IP - Input Process
static boolean traverseip(value subip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* ip, void* processes)

{     //find an input process
      boolean traverseip_result;
      switch (subip->tag) {
      case inprocessval:     traverseip_result=findop(processes, &subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip); break;
      case outprocessval:    traverseip_result=false; break;
      case choiceprocessval: if (traverseip(subip->p1, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes))  traverseip_result=true;
                        else traverseip_result=traverseip(subip->p2, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes);
                        break;
      case stopprocessval:   traverseip_result=false; break; //can't interact!
      case paraprocessval:   error("|| under |", lastid);
      break;
}   return traverseip_result;
}  

static boolean findip(value* ip, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals, void* processes)

{ boolean findip_result;
      if ((*ip)->tag==paraprocessval)    //p1||p2
           if (findip(&(*ip)->p1, lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes))  findip_result=true;
           else findip_result=findip(&(*ip)->p2, lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes);
      else findip_result=traverseip(*ip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes);  //p1|p2, a->p, stop
      return findip_result;
}  

static boolean interact(value* processes, void* lastid, integer* n, integer* channelcntr, integer* envcells, set* processvalues, integer* conscells, integer* evals)

{ boolean interact_result;
      interact_result = findip(processes, lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes);
      return interact_result;
}

static integer count(value p, set* processvalues, void* lastid)
{ integer count_result;
      { void* with = p; 
      if (inset(tag, *processvalues)) 
      switch (tag) {
         case paraprocessval:   count_result=count(p1, processvalues, lastid)+count(p2, processvalues, lastid); break;
         case choiceprocessval: count_result=ord(count(p1, processvalues, lastid)+count(p2, processvalues, lastid) > 0); break;
         case inprocessval: case outprocessval: count_result=1; break;
         case stopprocessval:   count_result=0;
         break;
      }
      else error("in count  ", lastid);}
      return count_result;
}

void eval( tree x, env rho , void* lastid, integer* channelcntr, integer* n, integer* envcells, set* processvalues, integer* conscells, integer* evals); 
void force( value v , void* lastid, integer* channelcntr, integer* n, integer* envcells, set* processvalues, integer* conscells, integer* evals);

void execute(tree prog, exec_ctx *c)
{
       int evals, envcells, conscells;  // statistics 
       alfa lastid;                  // debugging
       int64 processvalues;
       int channelcntr;
       value processes, outputchan, outputproc, inputchan, inputproc;
       tree outputmsg, inputmsg, outputcont, inputcont;
       env sysenv;
       integer n;

     evals = 0; envcells = 0; conscells = 0;    //zero counters
     lastid   = "start";
     channelcntr = 0;
     processvalues = setof(range(inprocessval,stopprocessval), eos);

     outputchan = mkchannel(&channelcntr, &n);
     outputmsg);
     {
                              void* with = outputmsg;   tag=ident; id="x         "; }
     outputcont);
     {
                               void* with = outputcont;   tag=ident; id="outputProc"; }
     outputproc = mkprocess2(inprocessval //!,
                             outputchan,outputmsg,outputcont,
                              nil);

     inputchan  = mkchannel(&channelcntr, &n);
     inputmsg); inputmsg->tag=emptycon;
     inputcont);
     {
                              void* with = inputcont;   tag=ident; id="inputProc "; }
     inputproc  = mkprocess2(outprocessval //!,
                             inputchan,inputmsg,inputcont,
                              nil);

     sysenv=bind("output    ", outputchan,
             bind("outputProc", outputproc,
             bind("input     ", inputchan,
             bind("inputProc ", inputproc,
             bind("stop      ", mkvalue(stopprocessval),
                  nil, &envcells), &envcells), &envcells), &envcells), &envcells);
     outputproc->pr=sysenv; inputproc->pr=sysenv;

     processes=mkprocess1(paraprocessval, outputproc,
                mkprocess1(paraprocessval, eval(prog, sysenv, &lastid, &channelcntr, &n, &envcells, &processvalues, &conscells, &evals),
                           inputproc, &lastid, &processvalues), &lastid, &processvalues);

     while (interact(&processes, &lastid, &n, &channelcntr, &envcells, &processvalues, &conscells, &evals)); ///the execution loop

     n=count(processes, &processvalues, &lastid);
     cwrite("\n"); cwrite("%i processes left",  n); if (n>2)  cwrite(" (deadlock)");
     cwrite("\n"); cwrite("%i evals, ",  evals);
     cwrite("%i env cells used, ",  envcells);
     cwrite("%i cells used\n",  conscells);
}
*/