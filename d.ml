
let ch=chan in let

 receiver =
   ch?chnl -> chnl!'A' ->                    
   ch?outAction ->                           
   outAction (                              
              ch?outX ->                  
              outX 'C' (                    
                        ch?inAction ->           
                        inAction (              
                                  lambda z.output!z -> 
              stop))),

 sender =
  let outputB = 
        lambda outContinuation. output!'B' -> outContinuation,

      outputX =
        lambda X. lambda outContinuation.
          output!X -> outContinuation,

      inputX  = 
        lambda inContinuation. ch?X -> inContinuation X

  in ch!output -> ch!outputB -> ch!outputX ->
     ch!inputX -> ch!'D' -> stop

in sender || receiver

