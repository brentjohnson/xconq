(game-module "ng-sides"
  (blurb "Generates random side names")
  )

;	Based on short-generic-names in ng-weird, but even shorter. 
;	No bipartite or trisyllabic names allowed.

(namer default-side-names (grammar root 10
  (root (or 50 (syllable syllable)
           25 (start vowel end)))
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
