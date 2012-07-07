let rec
   first = lambda n. lambda l.
      if n=0 then nil
      else (hd l)::(first (n-1) tl l),

   from = lambda n. n::(from (n+1))

in let rec
   filter = lambda f. lambda l. 
      if null l then nil
      else if hd l/f*f = hd l then filter f  tl l
      else hd l :: filter f  tl l,

   sieve = lambda l.
      if null l then nil
      else let p = hd l 
           in p :: sieve (filter  p  tl l)

in first 10 ( sieve (from 2) )

