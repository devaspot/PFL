let  Y = lambda G. (lambda g. G(g g)) (lambda g. G(g g)),
     x = 1,
     a = spawn let o = 1 in output ! 11 -> output ! 22 -> stop,
     b = spawn let p = 1 in output ! 33 -> output ! 33 -> stop,
     F = lambda f. lambda n. if n=0 then 1 else n * f(n-1)
in  output ! a -> output ! b -> stop || output ! Y F 10 -> stop || output ! 1 -> output ! 2 -> stop || output ! 3 -> output ! 4 -> stop
