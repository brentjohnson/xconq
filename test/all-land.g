(game-module "t-land"
  (variants (see-all true))
)

(set random-state 1)

(include "standard")

(include "ng-italian")

(add (town city) namer "italian-place-names")

;(set see-all true)

(set country-radius-min 4)

(set country-radius-max 30)

(add t* country-growth-chance 80)

(add town unit-growth-chance 100)

(add base unit-growth-chance 100)

(table favored-terrain add (base plains 100))

;(add town independent-growth-chance 200)

(table base-consumption (u* m* 0))

(table independent-density (u* t* 0))

(area 60 30)

(area (terrain
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "13e2a30e2a13e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
  "60e"
))
