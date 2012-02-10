
�������� ����������� ������
===========================

��������������� ������ ����������� ��������� ������������
����� ������ ���������� � ������������������ ���������� ����������� ������.
��� ����� ���� ��������� ��������������� � �������� ��� ���������.

       quote
       lambda
       application
       block
       expression
       decln
       declist
       ifexp
       matchexp

����� ����������� ������
========================

����� ���������� �������� ������������ ����� ������ ���������� ��������.
���� �������� ������������ �����...

       proc     .|., .||.
       ioproc   c?x->..., c!e->...
       value 
       list;
       func;

�������� ����������

struct exec_ctx {
   int evals, envcells, conscells; // statistics 
   alfa LastId;                 { debugging}
   int64 processvalues;
   int ChannelCntr;
   vaue Processes, OutputChan, OutputProc, InputChan, InputProc;
   tree OutputMsg, InputMsg, OutputCont, InputCont;
   env SysEnv;
   int n;
} exec_ctx;

��������� �����
===============

��� �������� ������� ��������� ML �������� ������������� ��������������� ����� ���������,
����������� ��� ������������ ���������� ���������. ������� � ����� �� ������������ �����
����������� ��� pattern matching � ������� �����.

����� �������, ������� ����� ������ ���������� ������������ ����� ������ �������������
����������� � ���������, ��� ���������� let rec.

program ::= exp
exp ::= ident | numeral | 'letter' | () | true | false | nil |
        ( exp ) | unopr exp | exp binopr exp |
        if exp then exp else exp |
        lambda param . exp  |  exp exp |
        let [rec] decs in exp
decs  ::= dec , decs | dec
dec   ::= ident = exp
param ::= () | ident
unopr ::= head | tail | null | not | -
binopr ::= and | or | = | <> | < | <= | > | >= | + | - | * | / | ::
priorities: ::                   1 cons list (right associative)
            or                   2
            and                  3
            = <> < <= > >=       4 scalars only
            + -                  5 (binary -)
            * /                  6
            application          7 {left associative, f x y = (f(x))(y)}
            - head tail null not 8 (unary -)

����� �����, ����� �������� �� ���������� ��������� ����������

processes:
  process1 || process2       p1 and p2 in parallel
  process1 |  process2       choice of either p1 or p2
  action -> process          sequence
  stop                       null process

���������� ��������� �������������� ������� ���������� ��������� || ����
��������� |. �������� ��������� (p1||p2)|p3 �������� ������������.

actions:
  channel ? variable         input  action
  channel ! expression       output action

channels:
  let c=chan in ...          chan returns a new channel
  output                     standard ouput channel
  input                      standard input channel
