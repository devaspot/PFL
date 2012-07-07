let rec
 merge = lambda a. lambda b.
   if hd a < hd b then (hd a) :: (merge tl a b)
   else if hd b < hd a then (hd b) :: (merge a tl b)
   else (hd a) :: (merge tl a tl b),

 mul = lambda n. lambda l. (n* hd l) :: (mul n tl l),
 hamm = 1 :: (merge (mul 2 hamm)
             (merge (mul 3 hamm)
                    (mul 5 hamm)))

in output ! hamm -> stop

