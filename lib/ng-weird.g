(game-module "ng-weird"
  (blurb "Generates random weird names")
  )

;;; This is a generic "pronounceable name" generator.
;;; Doesn't have any particular flavor, although the weights are
;;; tilted towards English.

(namer generic-names (grammar root 10
  (root (or 5 (syllable " " root)
           30 (syllable syllable)
           20 (syllable syllable syllable)
           50 (syllable root)))
  (syllable (or (start vowel) (vowel end) 3 (start vowel end)))
  (start (or 3 initdiph 7 consonant))
  (end (or 2 enddiph 8 consonant))
  (vowel (or 5 a 9 e 2 i 3 o 2 u y))
  (consonant (or
     2 b 2 c 3 d 2 f 2 g 3 h   j 2 k 3 l 3 m
     6 n 3 p 4 r 5 s 9 t 2 v 2 w   x       z))
  (initdiph (or 
    bl cl    fl gl kl pl sl
    br cr dr fr gr kr pr    3 tr wr 
    3 ch ph rh 3 sh 3 th wh
    2 sm 2 sp 4 st sw
    ))
  (enddiph (or
    lb ld lf lg lk ll lm ln lp    ls lt
    rb rd rf rg rk rl rm rn rp rr rs rt
    mp
    3 nd 2 ng nk 3 nt
    gh
    ))
))

(namer short-generic-names (grammar root 10
  (root (or 10 (syllable " " (capitalize root))
           50 (syllable syllable)
           5 (start vowel end)
           5 (syllable syllable syllable)
           5 (syllable root)))
  (syllable (or (start vowel) (vowel end) 3 (start vowel end)))
  (start (or 1 initdiph 7 initconsonant))
  (end (or 1 enddiph 8 endconsonant))
  (vowel (or 5 a 9 e 2 i 3 o 2 u y))
  (initconsonant (or
     2 b 2 c 3 d 2 f 2 g 3 h   j 2 k 3 l 3 m
     6 n 3 p 4 r 5 s 9 t 2 v 2 w         z))
  (endconsonant (or
     2 b 2 c 3 d 2 f 2 g      2 k 3 l 3 m
     6 n 3 p 4 r 5 s 9 t 2 v 2 w   x       z))
  (initdiph (or 
    bl cl    fl gl kl pl sl
    br cr 2 dr fr gr kr 2 pr    3 tr 2 wr 
    3 ch ph rh 3 sh 3 th wh
    2 sm 2 sp 4 st sw
    ))
  (enddiph (or
    ld lf lk ll lm ln lp    ls lt
    rb 2 rd rf rg rk rl rm rn rp rr rs 2 rt
    2 mp
    3 nd 2 ng nk 3 nt
    gh
    ))
))

