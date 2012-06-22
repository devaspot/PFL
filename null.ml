let rec
    run = lambda nn. output ! (spawn let o = 1 in o) -> if nn = 1 then stop else run (nn-1)
 in run 10000
