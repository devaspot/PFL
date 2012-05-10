// ML runtime

#include <string.h>
#include <stdlib.h>
#include "exec.h"

void  rterror(char *m, alfa lastid) { printf("\nError: %s LastId=%s\n", m, lastid); }
value checkval(char *m, values vs, value v, alfa lastid) { if (v->tag & vs == 0) rterror(m, lastid); return v; }
value mkvalue(valueclass t) { value p=(value)malloc(sizeof(valnode)); p->tag=t; return p; }
value mkint(int nn) { value p=(value)malloc(sizeof(valnode)); p->tag=intval; p->n=nn; return p; }
value mkbool(int bb) { value p=(value)malloc(sizeof(valnode)); p->tag=boolval; p->b=bb; return p; }
value mkchar(char cc) { value p=(value)malloc(sizeof(valnode)); p->tag=charval; p->ch=cc; return p; }
value mkfunc(tree code, env rho) { value p=(value)malloc(sizeof(valnode)); p->tag=funcval; p->func.e=code; p->func.r=rho; return p; }
value defer(tree x, env rho) { value p=(value)malloc(sizeof(valnode)); p->tag=deferval; p->func.e=x; p->func.r=rho; return p; }
value vcons(value h, value t, int *conscells) { value p=(value)malloc(sizeof(valnode)); p->tag=listval; p->list.hd=h; p->list.tl=t; *conscells++; return p; }

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

value mkchannel(int* channelcntr)
{
    value p=(value)malloc(sizeof(valnode));
    p->tag=channelval;  //choice or para, | or ||, NB. pA, pB in WHNF
    printf("channel %i created\n",*channelcntr);
    p->n = (*channelcntr)++;
    return p;
}

void force(value v, alfa lastid, int* channelcntr, int *n, int* envcells, int64 processvalues, int* conscells, int* evals);

env bind(alfa x, value val, env r, int* envcells) // :Ide -> Value -> Env -> Env 
{
    env p=(env)malloc(sizeof(binding));
    *envcells++;
    strcpy((char*)p->id,(char*)x);
    p->v=val;
    p->next=r;
    return p;
}

value applyenv(env r, alfa x, alfa lastid, int* channelcntr, int *n, int* envcells, int64 processvalues, int* conscells, int* evals)      /* :Env -> Ide -> Value */
{ 
    strcpy(lastid,x);
    if (r==0) rterror("undec id", lastid);
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
    else return bind(decs->declist.head->decln.name,
		     defer(decs->declist.head->decln.value,local),
		     d2(decs->declist.tail, local, global, envcells),
		     envcells);
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

value eval(tree x, env rho, alfa lastid, int* channelcntr, int *n, int* envcells, int64 processvalues, int* conscells, int* evals);

value apply(value fn, value ap, alfa lastid, int* envcells, int*n, int* channelcntr, int64 processvalues, int* conscells, int* evals) // apply a function fn to param ap
{
    if (fn->func.e->lambda.parm->tag == emptycon)   // (L().e)ap 
    {
        force(ap, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        if (ap->tag == emptyval) {
            return eval(fn->func.e->lambda.body, fn->func.r,
                        lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        }
        else 
        {
            rterror("L().e exp", lastid);
            return 0;
        }
    }
    else return eval(fn->func.e->lambda.body, bind(fn->func.e->lambda.parm->id, ap, fn->func.r, envcells), 
                     lastid, channelcntr, n, envcells, processvalues, conscells, evals);
}

value o(symbol opr, value v1, value v2, alfa lastid, int64 processvalues, int* conscells) // O :Value^2 -> Value 
{
    int abs1, abs2, intans;
    int boolans;
    value o_result;
    switch (opr)
    {
	case parallelsy: //...||...
	    o_result = mkprocess1(paraprocessval, v1, v2, lastid, processvalues);
	    break;
	case choicesy:   //...|.... 
	    o_result = mkprocess1(choiceprocessval, v1, v2, lastid, processvalues);
	    break;
	case eq: case ne: case lt: case le: case gt: case ge:
    	    if (1 << v1->tag && 1 << v2->tag && (1 << intval | 1 << charval | 1 << boolval) != 0)
	    switch (v1->tag)
	    {
		case intval: { abs1=v1->n; abs2=v2->n; } break;
		case boolval: { abs1=(int)(v1->b); abs2=(int)(v2->b); } break;
		case charval: { abs1=(int)(v1->ch); abs2=(int)(v2->ch); } break;
            }
            else
        	rterror("rel ops   ", lastid);
        	
            switch (opr)
            {
        	case eq: boolans=abs1==abs2; break; case ne: boolans=abs1!=abs2; break;
        	case le: boolans=abs1<=abs2; break; case lt: boolans=abs1< abs2; break;
        	case ge: boolans=abs1>=abs2; break; case gt: boolans=abs1> abs2; break;
            }
            o_result=mkbool(boolans);
            break;        
	case plus: case minus: case times: case over:
	    if ( (1 << v1->tag | 1 << v2->tag) == (1 << intval)) 
            switch (opr)
            {
        	case plus:  intans=v1->n + v2->n; break;
                case minus: intans=v1->n - v2->n; break;
                case times: intans=v1->n * v2->n; break;
                case over:  intans=v1->n / v2->n; break;
            }
            else rterror("arith opr ", lastid);
            o_result=mkint(intans);
            break;
        case andsy: 
        case orsy:
            if ((1 << v1->tag | 1 << v2->tag) == (1 << boolval)) 
               switch (opr) {
               case andsy: boolans=v1->b & v2->b; break;
               case orsy:  boolans=v1->b | v2->b;
               break;
               }
               else rterror("bool opr  ", lastid);
               o_result=mkbool(boolans);
    	    break;
        case conssy: // deferred params 
            o_result=vcons(v1, v2, conscells);
    	    break;
    }
    return o_result;
}

value u(symbol opr, value v, alfa lastid, int* channelcntr, int *n, int* envcells, int64 processvalues, int* conscells, int* evals)    
// U :Value -> Value 
//PRE: v^.tag <> deferval
{
    value u_result;
    switch (opr) {
	case minus: if (v->tag==intval) u_result=mkint(-v->n); else rterror("- non int ", lastid); break;
	case notsy: if (v->tag==boolval) u_result=mkbool(~ v->b); else rterror("not ~bool ", lastid); break;
	case hdsy:  if (v->tag==listval) { force(v->list.hd, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
					     u_result=v->list.hd; } else rterror("hd ~list  ", lastid); break;
	case tlsy:  if (v->tag==listval) { force(v->list.tl, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
					    u_result=v->list.tl; } else rterror("tl ~list  ", lastid); break;
        case nullsy:if (v->tag==listval) u_result=mkbool(0); else if (v->tag==nilval)  u_result=mkbool(1); else rterror("null ~list", lastid); break;
    }
    return u_result;
}

value eval(tree x, env rho, alfa lastid, int* channelcntr, int *n, int* envcells, int64 processvalues, int* conscells, int* evals)
// eval :Exp -> Env -> Value  Note: evaluates an Expression and returns a Value*
// POST: result tag is not deferval, weak head normal form
{
    value func, switch_, chnl;
    valueclass proctag;
    value eval_result;
    switch (x->tag) {
        case ident:     eval_result=applyenv(rho, x->id, lastid, channelcntr, n, envcells, processvalues, conscells, evals); break;
        case intcon:    eval_result=mkint(x->n); break;
        case boolcon:   eval_result=mkbool(x->b); break;
        case charcon:   eval_result=mkchar(x->ch); break;
        case nilcon:    eval_result=mkvalue(nilval); break;
        case emptycon:  eval_result=mkvalue(emptyval); break;
        case newchan:   eval_result=mkchannel(channelcntr); break;
        case lambdaexp: eval_result=mkfunc(x, rho); break;
        case application:
            func = eval(x->application.func, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
            if (func->tag==funcval) 
		eval_result = apply(func,
				    defer(x->application.parm, rho),
				    lastid, envcells, channelcntr, n, processvalues, conscells, evals);
				    
            else rterror("apply ~fn ", lastid);
            break;
        case unexp:   eval_result=u(x->expression.op, eval(x->expression.left, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals), lastid, channelcntr, n, envcells, processvalues, conscells, evals); break;
	case binexp:
	    if (x->expression.op==sequencesy)                   //->
            {
        	if (x->expression.left->tag == binexp) 
                {
            	    if (x->expression.left->expression.op == inputsy)         //...?...->
                    {
                	proctag=inprocessval;
                        if (x->expression.left->expression.right->tag != ident) 
                    	    rterror("...?~var  ", lastid);
                    }
                    else if (x->expression.left->expression.op == outputsy)   //...!...->
                    {
                        proctag=outprocessval;
                    } 
                    else rterror("~\?/! ->...", lastid);
                    
                    chnl=eval(x->expression.left->expression.left,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                    
                    if (chnl->tag != channelval) rterror("chan xpctd", lastid);
                    
                    eval_result=mkprocess2(proctag,chnl,x->expression.left->expression.right,x->expression.right,rho);
        	}
                else rterror("~IO -> ...", lastid);
             }
             else if ((x->expression.op & (1 << inputsy | 1 << outputsy)) != 0) 
             {
                     //An action is part of a process, not a Value, ?yet? 
                     // NB. an input action needs a Cont to take in-value  
                     rterror("eval ? | !", lastid);

	     }
             else if (x->expression.op==conssy) // cons should not eval ... 
             {
        	eval_result=o(x->expression.op,
        		      defer(x->expression.left,rho),
                              defer(x->expression.right,rho),
                              lastid, processvalues, conscells);
                              
	     }
             else eval_result=o(x->expression.op,
	    			eval(x->expression.left,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals),   
                                //others strict
                        	eval(x->expression.right,rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals),
                        	lastid, processvalues, conscells);
	    break;
	case ifexp:
            switch_=eval(x->ifexp.e1, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
            if (switch_->tag==boolval) 
            { 
                if (switch_->b) 
                     eval_result=eval(x->ifexp.e2, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
                else eval_result=eval(x->ifexp.e3, rho, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
            } else rterror("if ~bool  ", lastid);
            break;
	case block:
	    eval_result=eval(x->block.expr,
			     d(x->block.decs, rho, envcells),
			     lastid, channelcntr, n, envcells, processvalues, conscells, evals);
    	    break;
    }   
    *evals = *evals + 1; // statistics 
    return eval_result;
}

void force(value v, alfa lastid, int* channelcntr, int*n, int* envcells, int64 processvalues, int* conscells, int* evals)
{
    value fv=(value)malloc(sizeof(valnode));
    if (v->tag == deferval) 
    {
        fv = eval(v->func.e, v->func.r, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        *v = *fv;
    }
}

void showvalue(value v, alfa lastid, int* channelcntr, int*n, int* envcells, int64 processvalues, int* conscells, int* evals)
{
    switch (v->tag) {
	case intval:  printf("%1i",  v->n); break;
	case boolval: printf("%1i",  v->b ); break;
        case charval: printf("%c",  v->ch ); break;
        case emptyval:printf("{}" ); break;
        case nilval:  printf("nil"); break;
        case listval: { printf("{");
    			showvalue(v->list.hd, lastid, channelcntr, n,envcells, processvalues, conscells, evals); 
    			printf(",");
    			showvalue(v->list.tl, lastid, channelcntr, n,envcells, processvalues, conscells, evals); 
    			printf("}"); } break;
        case funcval: printf("function"); break;
        case inprocessval: case outprocessval: case choiceprocessval: case paraprocessval: printf("process"); break;
        case stopprocessval: printf("stop"); break;
        case channelval: printf("chan%1i", v->n); break;
        case deferval:{ force(v, lastid, channelcntr, n,envcells, processvalues, conscells, evals); 
    			showvalue(v, lastid, channelcntr, n,envcells, processvalues, conscells, evals); } break; // evaluation is o/p driven 
    }
}

int complementary(value *subip, value *subop, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals, value *op, value *ip)
{
    int chan;
    value val;
    //?can IP and OP interact?
    int complementary_result = 0; //unless
    chan = (*subip)->ioproc.chan->n;
    if (chan == (*subop)->ioproc.chan->n)  //exchange msg
    {
	complementary_result=1;
        if (chan==1)//input 
        {        //val:= ...
	    rterror("not impl  ", lastid);
        } else val=defer((*subop)->ioproc.msg,(*subop)->ioproc.pr);
        
        if (chan==0)  { 
           printf("\n"); printf(" output "); showvalue(val, lastid, channelcntr, n, envcells, processvalues, conscells, evals); 
        } else {
           printf(" ch%1i ", chan); showvalue(val, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        }

        *op=eval((*subop)->ioproc.cont,(*subop)->ioproc.pr, lastid, channelcntr, n, envcells, processvalues, conscells, evals);
        *ip=eval((*subip)->ioproc.cont,bind((*subip)->ioproc.msg->id,val,(*subip)->ioproc.pr, envcells), lastid, channelcntr, n, envcells, processvalues, conscells, evals);

    }
    return complementary_result;
}
                                                  //OP - Output Process
int traverseop(value *subop, value *subip, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals, value *op, value *ip)
{ 
    //find an output process
    int traverseop_result = 0;
    switch ((*subop)->tag) {
	case inprocessval: break;
	case outprocessval: 
             traverseop_result =
             complementary(subip, subop, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip); 
             break;
	case choiceprocessval:
	    if (traverseop(&((*subop)->proc.p1), subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip)) 
            {
               traverseop_result=1;
            }
            else {
               traverseop_result=traverseop(&((*subop)->proc.p2), subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip);
            }
            break;
	case stopprocessval: break; //can't interact!
	case paraprocessval: rterror("|| under |", lastid); break;
    }
    return traverseop_result;
}

int findop(value *op, value *subip, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals, value *ip)
{ 
    int findop_result;
    if ((*op)->tag==paraprocessval)   //p1||p2
    {
        if (findop(&((*op)->proc.p1), subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip))  
        {
            findop_result=1;
        }
        else {
            findop_result=findop(&((*op)->proc.p2), subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip);
        }
    } else 
    {
        findop_result=traverseop(op, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, op, ip); 
    }
    //p1|p2, a->p, stop
    return findop_result;
}
                                                  //IP - Input Process
int traverseip(value *subip, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals, value *ip, value *processes)
{     //find an input process
    int traverseip_result = 0;
    switch ((*subip)->tag)
    {
         case inprocessval: 
              traverseip_result=findop(processes, subip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip); break;
         case outprocessval: 
              break;
         case choiceprocessval: 
              if  (traverseip(&((*subip)->proc.p1), lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes))
              {
                   traverseip_result=1;
              } else traverseip_result=traverseip(&((*subip)->proc.p2), lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes);
              break;
         case stopprocessval: break; //can't interact!
         case paraprocessval: rterror("|| under |", lastid); break;
    }
    return traverseip_result;
}
  

int findip(value *ip, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals, value *processes)
{
    int findip_result = 0;
    if ((*ip)->tag==paraprocessval)    //p1||p2
    {
        if (findip(&((*ip)->proc.p1), lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes) == 1)
        {
            findip_result=1;
        }
        else { 
            findip_result=findip(&((*ip)->proc.p2), lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes);
        }
    }
    else 
    {
        findip_result=traverseip(ip, lastid, n, channelcntr, envcells, processvalues, conscells, evals, ip, processes);
    }
    //p1|p2, a->p, stop
     return findip_result;
}  

int interact(value *processes, alfa lastid, int* n, int* channelcntr, int* envcells, int64 processvalues, int* conscells, int* evals)
{ 
    int interact_result;
    interact_result = findip(processes, lastid, n, channelcntr, envcells, processvalues, conscells, evals, processes);
    return interact_result;
}

int count(value p, int64 processvalues, alfa lastid)
{
    int count_result;
    if ((p->tag && processvalues) != 0) 
    switch (p->tag) {
        case paraprocessval: count_result=count(p->proc.p1, processvalues, lastid)+count(p->proc.p2, processvalues, lastid); break;
        case choiceprocessval: count_result=(int)(count(p->proc.p1, processvalues, lastid)+count(p->proc.p2, processvalues, lastid) > 0); break;
        case inprocessval: case outprocessval: count_result=1; break;
        case stopprocessval: count_result=0; break;
    }
    else rterror("in count  ", lastid);
    return count_result;
}

void execute(tree prog, exec_ctx *c)
{
    int evals, envcells, conscells;  // statistics 
    alfa lastid;                  // debugging
    int64 processvalues;
    int channelcntr;
    value processes, outputchan, outputproc, inputchan, inputproc;
    tree outputmsg, inputmsg, outputcont, inputcont;
    env sysenv;
    int n;

    evals = 0; envcells = 0; conscells = 0;    //zero counters
    strcpy(lastid,"start     ");
    channelcntr = 0;
    processvalues = 1 << inprocessval |
                    1 << outprocessval | 
                    1 << choiceprocessval |
                    1 << paraprocessval |
                    1 << stopprocessval;

    outputchan = mkchannel(&channelcntr);
    outputmsg=(tree)malloc(sizeof(node));
    outputmsg->tag=ident;
    strcpy(outputmsg->id,"x         "); 
    outputcont=(tree)malloc(sizeof(node));
    outputcont->tag=ident;
    strcpy(outputcont->id,"outputProc"); 
    outputproc = mkprocess2(inprocessval,outputchan,outputmsg,outputcont,(env)0);
    inputchan  = mkchannel(&channelcntr);
    inputmsg=(tree)malloc(sizeof(node));
    inputmsg->tag=emptycon;
    inputcont=(tree)malloc(sizeof(node));
    inputcont->tag=ident;
    strcpy(inputcont->id,"inputProc ");
    inputproc  = mkprocess2(outprocessval,inputchan,inputmsg,inputcont,(env)0);

    sysenv= bind("output    ", outputchan,
	    bind("outputProc", outputproc,
	    bind("input     ", inputchan,
            bind("inputProc ", inputproc,
            bind("stop      ", mkvalue(stopprocessval),
    	    (env)0,
            &envcells), &envcells), &envcells), &envcells), &envcells);
    	    
     outputproc->ioproc.pr=sysenv;
     inputproc->ioproc.pr=sysenv;

     processes =
         mkprocess1(
               paraprocessval,
               outputproc,
               mkprocess1(
                   paraprocessval,
                   eval(prog, sysenv,
                        lastid, &channelcntr, &n, &envcells,
                        processvalues, &conscells, &evals),
                   inputproc,
                   lastid,
                   processvalues),
              lastid,
              processvalues);

     while (interact(&processes, lastid, &n, &channelcntr,
                     &envcells, processvalues, &conscells, &evals));

     n=count(processes, processvalues, lastid);

     printf("\n"); printf("%i processes left",  n); if (n>2) printf(" (deadlock)");
     printf("\n"); printf("%i evals, ",  evals);
     printf("%i env cells used, ",  envcells);
     printf("%i cells used\n",  conscells);
}
