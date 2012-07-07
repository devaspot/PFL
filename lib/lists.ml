let
    pair = lambda x. lambda y. x :: y :: nil,
    fst  = lambda xy. hd xy,
    snd  = lambda xy. hd tl xy,
    plus  = lambda x. lambda y. x+y,
    minus = lambda x. lambda y. x-y,
    times = lambda x. lambda y. x*y,
    over  = lambda x. lambda y. x/y,
    lt  = lambda x. lambda y. x <  y,
    le  = lambda x. lambda y. x <= y,
    gt  = lambda x. lambda y. x >  y,
    ge  = lambda x. lambda y. x >= y,
    eq  = lambda x. lambda y. x =  y,
    ne  = lambda x. lambda y. x <> y,
    even = lambda n. (n/2)*2=n,
    odd  = lambda n. not(even n),
    compose = lambda p. lambda q. lambda x. p(q x),
    cons = lambda H. lambda T. H::T 

in 

let rec take = lambda n. lambda l.
        if n < 1  then nil else
        if null l then nil else
        hd l :: take (n-1) tl l,

        append = lambda L1. lambda L2.
        if null L1 then L2
        else (hd L1) :: (append (tl L1) L2),

        zip = lambda L1. lambda L2.
        if null L1 or null L2 then nil
        else (pair hd L1 hd L2)::(zip tl L1 tl L2),

        merge = lambda L1. lambda L2.
        if null L1 then L2
        else if null L2 then L1
        else if hd L1 < hd L2 then
        hd L1 :: merge tl L1 L2
        else hd L2 :: merge L1 tl L2,

        reverse = lambda L. let rec rev = lambda inp. lambda op.
        if null inp then op
        else rev (tl inp) ((hd inp)::op)
        in rev L nil,

        map = lambda f. lambda L.
        if null L then nil else (f hd L)::(map f tl L),

        foldl = lambda f. lambda z. lambda L.
        let rec ff = lambda inp. lambda ans.
        if null inp then ans
        else ff tl inp (f ans hd inp)
        in ff L z,

        foldr = lambda f. lambda z. lambda L.
        let rec ff = lambda L.
        if null L then z
        else f hd L (ff tl L)
        in ff L

in
let rec
  L1 = append (1::3::5::nil) (7::9::11::nil),
  L2 = reverseS L1,
  L3 = reverse  L2,
  L4 = zip L1 L3,
  L5 = map fst L4,
  L6 = merge L5 (2::4::6::8::10::nil)
in L6
