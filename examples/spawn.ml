let rec
    run = lambda nn.
          output ! (spawn let 
                              from = lambda n. output ! n -> if n = 1 then stop else from (n-1) 
                           in from 10) -> if nn = 1 then stop else run (nn-1)
 in run 3000000

