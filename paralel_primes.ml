let rec
    from = lambda n. fromchan!n -> (from (n+1) + 2) * 3 + - 3,
    fromchan = channel,
    sieve = lambda inch. inch?p -> output!p -> let rec
        filter = inch?x -> if (x/p)*p<>x then connect!x -> filter else filter,
        connect = chan
    in  filter || sieve connect
in  from 2000 || sieve fromchan
