;; This is the game used by "make check-auto".
;; In the future, individual tests within autotest probably will
;; want to set up game stuff (using designer mode or something similar?),
;; but for the moment this seems easier.
(game-module "autotest")

(terrain-type plains (char "+"))

(area 5 5)

(area (terrain (by-name "+" plains) "5+" "5+" "5+" "5+" "5+"))

(unit-type town (image-name "town20") (char "*"))
(unit-type fighter (char "f"))
(unit-type bomber (char "b"))
(unit-type infantry (char "i"))

(add u* acp-per-turn 1)

(table tp-crossover
 (fighter bomber 75)
)
(table tp-max
 (town (fighter bomber) (10 20))
)

(add t* capacity 16)
(table unit-size-in-terrain (u* t* 1))

(add bomber capacity 1)
(add town capacity 50)
(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  (infantry bomber 1)
  ((fighter bomber infantry) town 1)
  )

(add u* acp-per-turn 1)
