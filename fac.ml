let rec
   factorial
      = lambda n.
        if n=0 then 1
               else n*factorial(n-1)

in output ! factorial 4 -> stop

