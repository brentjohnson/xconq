(game-module "hill"
  (title "King of the Hill")
  (blurb "Be 10 years old again!")
  (variants (world-seen true) (see-all true) (sequential false))
)

;(set see-all true)

(unit-type kid (image-name "person") (help "a participant"))

(terrain-type grass (image-name "plains") (char "+"))
(terrain-type hill (image-name "hill") (char "^"))

(add kid acp-per-turn 2)

(table mp-to-enter-terrain (kid hill 2))

(add kid hp-max 99)

;; The parameters here simulate kids shoving each other.

(table withdraw-chance-per-attack (kid kid 50))

(table hit-chance (kid kid 90))

(table damage (kid kid 1))

(table counterattack (kid kid 0))

;; Harder to get at a kid that's on the hill.

(table defend-terrain-effect (kid hill 70))

;; Get any hp losses back always.

(add kid hp-recovery 99.00)

(set action-notices '(
  ((hit kid kid) (actor " shoves " actee "!"))
  ))

(set meridians false)

(game-module (notes (
  "This period `implements' 10-year-old kids.  They can hit each other,"
  "but damage is very minor, and soon recovered from.  The most likely result"
  "is for one or both to run away."
  )))

(area 30 21 (terrain
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "15ab14a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
))

(set sides-max 2)

(side 1 (plural-noun "Garbage Pail Kids"))

(side 2 (plural-noun "Deadend Kids"))

(kid (n "Tommy") (@ 7 10) (s 1))
(kid (n "Jenny") (@ 7 9) (s 1))
(kid (n "Stanley") (@ 7 11) (s 1))
(kid (n "Barry") (@ 6 10) (s 1))
(kid (n "Charlie") (@ 8 10) (s 1))
(kid (n "Mikey") (@ 6 11) (s 1))
(kid (n "Marty") (@ 8 9) (s 1))
(kid (n "Cathy") (@ 9 10) (s 1))
(kid (n "Joey") (@ 23 10) (s 2))
(kid (n "Leslie") (@ 24 9) (s 2))
(kid (n "Timmy") (@ 22 11) (s 2))
(kid (n "Harry") (@ 24 10) (s 2))
(kid (n "Bobby") (@ 22 10) (s 2))
(kid (n "Julie") (@ 23 11) (s 2))
(kid (n "Jimmy") (@ 23 9) (s 2))
(kid (n "Patti") (@ 21 10) (s 2))

;;; The scorekeeper (should) gives the win to the occupier of the hill after
;;; some number of turns has passed.
