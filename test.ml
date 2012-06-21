let  Y = lambda G. (lambda g. G(g g)) (lambda g. G(g g)),
     F = lambda f. lambda n. if n=0 then 1 else n * f(n-1)
in   output ! Y F 10 -> stop
