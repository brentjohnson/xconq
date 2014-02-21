;;; German-like place name generator.

;;; Conventional combos most common, random syllables rare.

(namer german-place-names (grammar root 100
  (root (or 95 (name)
             5 ("Bad " name)
	     2 (name " am See")
	     2 (name " am Wald")
	     1 (name " an der " (capitalize syll))
	     1 (name "-" name)
	     ))
  (name (capitalize nam1))
  (nam1 (or 40 (prefix suffix)
            20 (prefix both)
	     3 (prefix both suffix)
	     3 (syll both suffix)
	    10 (syll suffix)
	     5 (prefix syll suffix)
	))
  (prefix (or 40 (prefixa) 20 (prefixb "en") 5 (proper "s")))
  (proper (or karl friedrich ludwig hermann))
  (prefixa (or
	schwarz blau gruen gelb rot roth braun weiss
	wolf schwein ross
	neu alt
	salz kloster muenster kirch muehl
	hoch ueber nieder gross klein
	west ost nord sued
	;; from real names
	frank duessel mut
	))
  (prefixb (or
	schwarz rot roth weiss
	wolf ochs eich buch
	kirch muehl
	neu alt 
	hoh gross herr
	;; from real names
	frank
	))
  (suffix (or
	dorf torf heim holz hof hofen burg stedt stetten
	bruck brueck bach tal thal furt reuth rode rath hafen
	beuren bueren buettel trop hall kirch kirchen bergen
	mund muende weier weiler haus hausen muehle
	;; these aren't so great
	ach ingen nitz
	))
  (both (or
	feld stadt stein see schloss wasser eisen berg heide au
	))
  (syll (or 40 (startsyll vowel endsyll) 5 (vowel endsyll)))
  (startsyll (or 30 startcons 10 startdiph))
  (startcons (or b k d f g l m n r 5 s 3 t))
  (startdiph (or bl kl fl gl sl 3 sch 2 schl
                 br dr kr fr gr 2 schr 3 tr 2 th 2 thr))
  (vowel (or 6 a ae 2 au 5 e 2 ei 2 ie 6 i 3 o oe 2 u ue))
  (endsyll (or 4 b 5 l 3 n 4 r 4 t
               bs ls ns rs ts
               3 ch 3 ck
               lb lck lch lk lz ln lt lth ltz
               rb rck rch rn rt rth rtz
               ss sz
               2 th tz
  ))
))

; cons vowel cons [vowel] (usually en em es)

; cons vowel (or ss rr ll) vowel [cons]

;;; Should include a personal name generator that will do "Jacob von
;;; Dittersdorfnachspieleruntermunter... von Ulm" :-)

