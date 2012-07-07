let rec
    Y = lambda G. (lambda g. G(g g)) (lambda g. G(g g)),
    F = lambda f. lambda n. if n=0 then 1 else n*f(n-1),
    gen1 = gch!1 -> gch!2 -> gch!3 -> output ! Y F 8 -> stop,
    gen2 = gch!4 -> gch!5 -> gch!6 -> stop,
    gen3 = gch!7 -> gch!8 -> gch!9 -> stop,
    gch = chan,
    listen = lambda inch. 
        inch ? p -> output!p ->
        let rec filter = inch?x -> connect!x -> filter,
                connect = chan
             in filter || listen connect

 in gen1 || listen gch || gen3 || gen2


