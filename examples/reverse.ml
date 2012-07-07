let rec
        reverse = lambda L. let rec rev = lambda inp. lambda op.
        if null inp then op
        else rev (tl inp) ((hd inp)::op)
        in rev L nil,

  L1 = (1::3::5::7::9::11::nil),
  L2 = reverse L1
in output ! L2 -> stop
