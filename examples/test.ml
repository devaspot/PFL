let  Y = lambda G. (lambda g. G(g g)) (lambda g. G(g g)),
     x = 1,
     a = spawn let o = 1 in output ! 11 -> output ! 12 -> output ! 13 -> stop,
     b = spawn let p = 1 in output ! 21 -> output ! 22 -> output ! 23 -> stop,
     F = lambda f. lambda n. if n=0 then 1 else n * f(n-1)
in  output ! a -> output ! b -> stop || output ! Y F 10 -> stop || output ! 31 -> output ! 32 -> stop || output ! 33 -> output ! 34 -> stop
