(game-module "t-land"
  (variants
   (see-all true)
   (no-roads
    )
   (no-people
    (true
     (add t* country-people-chance 0)
     (add t* independent-people-chance 0)
     ))
   )
)

(set random-state 1)

(include "standard")

(include "ng-italian")

(add (town city) namer "italian-place-names")

(add town start-with 1)

;(set see-all true)

(set country-radius-min 4)

;(set country-radius-max 100)
;(add t* country-growth-chance 80)

(add t* country-people-chance 100)
(add t* independent-people-chance 100)

;(add town unit-growth-chance 100)
;(add base unit-growth-chance 100)

(table independent-density (u* t* 0))

(add road subtype-x 0)

(area 200 110)

(area (terrain
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "83e2a30e2a83e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"

  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
  "200e"
))
