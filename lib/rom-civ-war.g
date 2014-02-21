(game-module "rom-civ-war"
  (title "Roman Civil War")
  (version "1.0")
  (blurb "The Civil War: Caesar versus Pompeius in 48 BC.")
  (base-module "roman")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
  )
  (instructions 
   ("Cross the sea to destroy your enemy at once," 
    "or build an empire and wage a war of attrition."
    ))
  (notes 
   ("Situation in Jan 48 BC (706 AUC)."
    ""
    "Caesar has more legions,"
    "Pompeius has more auxiliaries and a bigger fleet."
    "Other sides should be machine-played."
    ""
    "Massimo Campostrini <Massimo.Campostrini@df.unipi.it>"
    ))
  )

; double production times of "roman"
;            i  c  a  L  B  T  U  S  _ X V C @ 
(add u* cp (24 60 40 80 64 96 48 40 32 1 1 1 1))

(set synthesis-methods nil)

(add u* start-with 0)

(set sides-min 4)
(set sides-max 4)

(side 1 (name "Caesar") (class "roman")  (color "blue"))
(side 2 (name "Pompeius") (class "roman")  (color "red"))
(side 3 (name "Parthia") (adjective "Parthian")
      (class "civilized") (color "dim-gray"))
(side 4 (name "Aegyptus") (adjective "Aegyptian") 
      (class "roman") #| yes, I do mean Roman |# (color "violet-red"))

(set see-all true) ; until variants impled when "opening".

(include "t-roman")

(set initial-date "1 Jan 706") ; Ab Urbe Condita, i.e. 48 BC
(set calendar '(usual week))

; ---------- Caesar

(unit "Brundisium" (s 1)) ; @ 107,40
(unit "Tarentum" (s 1))   ; @ 104,40
(L 109 39 0 (s 1))
(L 109 39 0 (s 1))
(L 109 38 0 (s 1))
(L 109 38 0 (s 1))
(L 105 41 0 (s 1))
(L 105 41 0 (s 1))
(L 105 40 0 (s 1))
(L 105 40 0 (s 1))
(L 106 40 0 (s 1))
(L 106 40 0 (s 1))
(L 108 39 0 (s 1))
(L 108 39 0 (s 1))
(c 103 41 0 (s 1))
(c 103 41 0 (s 1))
(c 104 41 0 (s 1))
(c 104 41 0 (s 1))
(i 107 39 0 (s 1))
(i 107 39 0 (s 1))
(i 107 39 0 (s 1))
(i 107 39 0 (s 1))
(a 103 41 0 (s 1))
(a 103 41 0 (s 1))
(a 104 41 0 (s 1))
(a 104 41 0 (s 1))
(T 106 39 0 (s 1))
(T 106 39 0 (s 1))
(T 105 39 0 (s 1))
(T 105 39 0 (s 1))
(B 104 40 0 (s 1) (in "Tarentum"))
(B 104 40 0 (s 1) (in "Tarentum"))
(B 105 39 0 (s 1))
(B 106 38 0 (s 1))
(B 106 39 0 (s 1))
(B 107 40 0 (s 1) (in "Brundisium"))
(U 104 40 0 (s 1) (in "Tarentum"))
(U 104 40 0 (s 1) (in "Tarentum"))
(U 104 40 0 (s 1) (in "Tarentum"))
(U 105 39 0 (s 1))
(U 106 39 0 (s 1))
(U 107 40 0 (s 1) (in "Brundisium"))

; ---------- Pompeius

(unit "Dyrrachium" (s 2)) ; @ 109,44
(unit "Nicopolis" (s 2)) ;  @ 115,37
; there are two oppida named Nicopolis, this gets the right one
(L 109 45 0 (s 2))
(L 110 43 0 (s 2))
(L 110 42 0 (s 2))
(L 111 41 0 (s 2))
(L 111 40 0 (s 2))
(L 112 40 0 (s 2))
(L 113 39 0 (s 2))
(L 113 38 0 (s 2))
(L 114 38 0 (s 2))
(i 110 43 0 (s 2))
(i 110 42 0 (s 2))
(i 111 41 0 (s 2))
(i 111 40 0 (s 2))
(i 112 40 0 (s 2))
(i 113 39 0 (s 2))
(i 113 38 0 (s 2))
(i 114 38 0 (s 2))
(a 110 43 0 (s 2))
(a 110 42 0 (s 2))
(a 111 41 0 (s 2))
(a 111 40 0 (s 2))
(a 112 40 0 (s 2))
(a 113 39 0 (s 2))
(a 113 38 0 (s 2))
(a 114 38 0 (s 2))
(c 111 43 0 (s 2))
(c 111 42 0 (s 2))
(c 112 41 0 (s 2))
(c 113 40 0 (s 2))
(c 114 39 0 (s 2))
(c 115 38 0 (s 2))
(T 110 40 0 (s 2))
(T 110 40 0 (s 2))
(T 111 39 0 (s 2))
(T 111 39 0 (s 2))
(T 112 38 0 (s 2))
(T 112 38 0 (s 2))
(T 113 37 0 (s 2))
(T 113 37 0 (s 2))
(B 110 40 0 (s 2))
(B 110 40 0 (s 2))
(B 111 39 0 (s 2))
(B 111 39 0 (s 2))
(B 112 38 0 (s 2))
(B 112 38 0 (s 2))
(B 113 37 0 (s 2))
(B 113 37 0 (s 2))
(U 109 44 0 (s 2) (in "Dyrrachium"))
(U 109 44 0 (s 2) (in "Dyrrachium"))
(U 109 44 0 (s 2) (in "Dyrrachium"))
(U 115 37 0 (s 2) (in "Nicopolis"))
(U 112 39 0 (s 2))
(U 112 39 0 (s 2))
(U 112 39 0 (s 2))
(U 112 39 0 (s 2))

#|
; Pompeians in 49 BC
; Afranius
(L 56 44 0 (s 2))
(L 57 44 0 (s 2))
(L 58 43 0 (s 2))
(i 56 44 0 (s 2))
(i 56 44 0 (s 2))
(i 57 44 0 (s 2))
(i 57 44 0 (s 2))
(c 58 43 0 (s 2))

; Varro
(L 38 40 0 (s 2))
(L 39 40 0 (s 2))
(i 38 40 0 (s 2))
(i 39 40 0 (s 2))
(c 39 40 0 (s 2))

; Petreius
(L 46 32 0 (s 2))
(L 46 33 0 (s 2))
(i 46 32 0 (s 2))
|#

; ---------- interested third parties
(unit "Ecbatana" (s 3))

(unit "Alexandria" (s 4))
