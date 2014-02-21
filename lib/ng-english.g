;;; Ye Olde English name generator.

;;; It usually composes from a set of stock syllables that occur a lot (just
;;; look at an atlas), with the occasional totally random syllable thrown in.
;;; Sometimes the syllables are *too* random or else in an inappropriate
;;; context, but fixing that would require some context-sensitivity in the
;;; grammar.

(game-module "ng-english"
  (blurb "Random English place name synthesizer")
  )

;;; just a test that other namers can be nonterminals
(namer english-metro-names (grammar root 9
  (root (or 50 ("city of " english-place-names)
	    50 ("Metropolitan " english-place-names)))
  ))

(namer english-place-names (grammar root 9
  (root (or 90 (word)
	    10 (prefix-word (capitalize word))
	     5 (word connective (capitalize word))
	))
  (word (or 60 (prefix suffix)
	    15 (prefix both)
	    40 (syllable suffix)
	    15 (both suffix)
;	    5 (both "ing" suffix)
	))
  (prefix (or
	2 south 2 north 2 east 2 west
	;; probably not any yellow,orange,purple towns in England
	3 black blue 2 green red brown gray 3 white
	peter whit wash ; what else?
	))
  (prefix-word (or "lower " "upper " "new " "old " "great " "little "))
  ;;; s sometimes inserted randomly
  (suffix (or
	borough bury
	cester chester
	field ford
	gate
	hall ham hill house
	ing ington ingston
	land lea ley ly
	minster mouth
	ney
	port
	sea sey stable stead stone ; "sy" doesn't fit well, where to do it?
	toke ton
	well wich worth
	))
  (both (or
	brook field kirk water
	))
  (connective (or 2 "-upon-" 4 "-on-" "-in-" "-over-" "-under-"))

  (syllable (or (start vowel) (vowel end) 3 (start vowel end)))
  (start (or 3 initdiph 7 startcons))
  (end (or 4 enddiph 4 endcons))
  (vowel (or 5 a 9 e 2 i 3 o 2 u y ey))
  (startcons (or
     2 b 2 c 3 d 2 f 2 g 3 h   j 2 k 3 l 3 m
     6 n 3 p 4 r 5 s 9 t     2 w            ))
  (endcons (or
     3 d 2 g 3 l 3 m
     6 n 4 r 5 s 9 t     2 w            ))
  (initdiph (or 
    bl cl    fl gl pl sl
    br cr dr fr gr pr    3 tr wr 
    3 ch 3 sh 3 th wh
    2 sm 2 sp 4 st sw
    ))
  (enddiph (or
    ld lf lk ll lm ln lp    ls lt
    rd rf rk rl rm rn rp rs rt
    mp
    3 nd 2 ng nk 3 nt
;    gh  ; hard to say...
    ))
  ))

; cons vowel (or r l) vowel cons  - as in "Salis"
