
LML with CCS
============

Lazy ML with Calculus of Communicating Systems (introduced by Robin Milner).
Язык который предназначен для образовательных целей, что бы можно было
"на пальцах" показать, как создать функциальный язык основанный на λ-исчислении.
Для перчинки в язык инкапсулированы ленивость и исчисление процессов,
предложенное Робином Милнером.

Публикация
----------

1. Максим Сохацкий. <a href="https://tonpa.guru/stream/2012/2012-06-22%20%D0%9B%D0%B5%D0%BD%D0%B8%D0%B2%D1%8B%D0%B9%20ML%20%D1%81%20pi-calculus%20%D0%B2%2026%D0%9A%D0%91.htm">Ленивый ML с π-calculus в 26КБ</a>, 2012.

Библиография
------------

[1]. S. Holmstrom. PFL: a functional language for parallel programming and its implementation. 1983
[2]. K. Mitchell. A user's guide to PFL. 1984
[3]. L. Allison. <a href="http://www.allisons.org/ll/FP/LambdaCCS/">The PFL interpreter</a>. 1993

Операции виртуальной машины
---------------------------

Сериализованное дерево исполняемой программы представляет
собой дерево хранящееся в последовательности операторов виртуальной машины.
Оно может быть полностью декомпилировано в исходный код программы.

    quote
    lambda
    application
    block
    expression
    decln
    declist
    ifexp
    matchexp

Среда виртуальной машины
------------------------

Среда именованых значений представляет собой список именованых значений.
Само значение представляет собой...

    proc     | , ||
    ioproc   c ? x -> ..., c ! e -> ...
    value    .
    list
    func

Контекст исполнения
-------------------

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

Выражения языка
---------------

При описании базовой структуры ML подязыка использовался минималистичный набор выражений,
необходимый для демонстрации исчисления процессов. Поэтому в языке не реализованый такие
возможности как pattern matching и система типов.

Таким образом, базовая часть лямбда исчисления представляет собой только фунциональные
определения и выражение, так называемый let rec.

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
    priorities:
	    ::                   1 cons list (right associative)
	    or                   2
	    and                  3
	    = <> < <= > >=       4 scalars only
	    + -                  5 { binary - }
	    * /                  6
	    application          7 { left associative, f x y = (f(x))(y) }
	    - head tail null not 8 { unary - }

Часть языка, котая отвечает за исчисление процессов моделирует

    processes:
        process1 || process2       p1 and p2 in parallel
        process1 |  process2       choice of either p1 or p2
        action -> process          sequence
        stop                       null process

Существует известное ограничивающее правило находжения оператора || ниже
оператора | Например выражение (p1||p2)|p3 является недопустимым.

    actions:
        channel ? variable         input  action
        channel ! expression       output action

    channels:
        let c=chan in ...          chan returns a new channel
        output                     standard ouput channel
        input                      standard input channel

Максим Сохацкий (maxim@synrc.com)
