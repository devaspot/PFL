let rec
 from = lambda n. fromchan ! n -> if n = 10 then stop else from (n+1),
 fromchan = chan,
 sieve = lambda inch.
   inch ? p -> output ! p ->
   let rec
     filter = inch?x ->
       if (x/p) * p <> x then connect!x -> filter
                     else filter,
     connect = chan
   in (filter || sieve connect)
in  from 2 || sieve fromchan
