let rec
    COPY   = input ? c -> output -> c,
    I      = lambda x. x,
    K      = lambda x. lambda y. x,
    S      = lambda x. lambda y. lambda z. (x z (y z)),
    mega   = lambda x. (x x),
    OMEGA  = mega mega,
    TRUE   = lambda x. lambda y. x,
    NULL   = lambda L. L (lambda h. lambda t. FALSE),
    MONUS  = lambda a. lambda b. b PRED a,
    DIVMOD = lambda x. lambda y.
              let rec dm = lambda q. lambda x.
                if LE y x then
                  dm (SUCC q) (MONUS x y)
                else pair q x
              in dm ZERO x,
    DIV    = lambda x. lambda y. DIVMOD x y fst,
    MOD    = lambda x. lambda y. DIVMOD x y snd,
    PAIR   = lambda fst. lambda snd. lambda sel. sel fst snd,
    PLUSb  = lambda x. x SUCC,
    FALSE  = ZERO,
    ZERO   = lambda s. lambda z. z,
    ONE    = lambda s. lambda z. s(z),
    TWO    = lambda s. lambda z. s(s(z)),
    PLUS   = lambda x. lambda y. lambda s. lambda z. x s (y s z),
    TIMES  = lambda x. lambda y. x (PLUS y) ZERO,
    PP     = lambda n. PRED (PRED n),
    PRED   = lambda n. lambda f. lambda x. n (lambda g. lambda h.h (g f)) (lambda u.x) (lambda u.u),
    ISZERO = lambda n. n (lambda x. ZERO) TRUE,
    IF     = lambda p. lambda a. lambda b. p a b,
    PRINT  = lambda n. n (lambda m. 'I'::m) '.',
    CONS   = lambda h. lambda t. lambda f. f h t,
    NIL    = lambda f. true,
    NULL   = lambda L. L (lambda h. lambda t. false),
    HD     = lambda L. L (lambda h. lambda t. h),
    TL     = lambda L. L (lambda h. lambda t. t),
    PRINT2 = lambda L. if NULL L then output ! '/' else (HD L)::(PRINT2 (TL L)),
    Y      = lambda G. (lambda g. G(g g)) (lambda g. G(g g)),
    FAC    = lambda f. lambda n. if n=0 then 1 else n*f(n-1),
    FIB    = lambda f. lambda n. IF (ISZERO (PRED n)) ONE IF (ISZERO (PP n)) ONE (PLUS (f(PRED n))(f(PP n))),
    FOUR   = MONUS (TIMES THREE TWO) (PLUS ONE ONE),
    EIGHT  = PLUSb FOUR FOUR
in
output ! PRINT (Y FIB (TIMES TWO TWO)) -> stop


  
