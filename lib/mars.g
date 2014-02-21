(game-module "mars"
  (title "Mars")
  ; (version "1.0")
  (blurb "Colonize Mars.")
  (variants
    (world-seen true)
    (see-all false)
    (sequential false)
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  ))

(terrain-type plani (image-name "orange") (char "+"))
(terrain-type highlands (image-name "brown") (char "^"))
(terrain-type canyon (image-name "tan"))
(terrain-type co2-ice (image-name "white"))
(terrain-type water-ice (image-name "light-blue"))

(terrain-type wall (subtype border) (image-name "brown"))

(define cell-t* (plani highlands))

(unit-type rover (image-name "hovercraft") (char "c")
  (help "slow, long-range"))
(unit-type boulder-car (image-name "hovercraft") (char "b")
  (help "fast, small, shorter-range, hard for satellites to see"))

(define ground-types (rover boulder-car))

(unit-type spy-satellite (image-name "spysat")
  (help "hovers overhead, observes all below"))

(define air-types (spy-satellite))

(unit-type base (image-name "village")
  )
(unit-type underground-city (image-name "village")
  )
(unit-type domed-city
  (help "large but vulnerable"))
(unit-type underice-city (image-name "village")
  (help "nearly invisible, can only be placed under water ice"))

(define city-types (underground-city domed-city underice-city))

(define place-types (base underground-city domed-city underice-city))

(material-type air)
(material-type water)
(material-type food)

;;; Static relationships.

(table vanishes-on
  (underice-city t* true)
  (underice-city water-ice true)
  )

(add place-types capacity 32)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  (ground-types place-types 1)
  (air-types place-types 1)
  )

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (place-types t* 8)
  )

;;; Vision.

;; The surface of Mars is well-known.

(set terrain-seen true)

;;; Actions.

(add ground-types acp-per-turn 8)

(add air-types acp-per-turn 4)

(add place-types acp-per-turn 1)

;;; Movement.

(add place-types speed 0)

;;; Construction.

(add u* cp (4 4 10 4 12 24 48))

(table can-create
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  (base base 0)
  )

(table acp-to-create
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  (base base 0)
  )

(table cp-on-creation
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  )

(table can-build
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  (base base 0)
  )

(table acp-to-build
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  (base base 0)
  )

(table cp-per-build
  (city-types ground-types 1)
  (city-types air-types 1)
  (base city-types 1)
  )

;;; Combat.

(add u* hp-max (1 1 1 10 20 20 20))

(table acp-to-attack
  (u* u* 1)
  (spy-satellite u* 0)
  )

(table hit-chance
  (u* u* 50)
  (spy-satellite u* 0)
  )

(table damage
  (u* u* 1)
  (spy-satellite u* 0)
  )

;;; Random game setup.

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 200)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add cell-t* alt-percentile-min (  0  70))
(add cell-t* alt-percentile-max ( 69 100))
(add cell-t* wet-percentile-min (  0   0))
(add cell-t* wet-percentile-max (100 100))

(add u* start-with 1)

(area 60 30)

(world 400)

(scorekeeper (do last-side-wins))

(game-module (notes (
  "This game was inspired by Kim Stanley Robinson's `Red Mars',"
  "an excellent story of the near-future colonization of Mars."
  ""
  "The general idea of the game is to build up a colony and perhaps"
  "eventually contend for control of Mars, if players can't work out"
  "agreements."
  )))

(game-module (design-notes (
  "This game is far from complete."
  ""
  "Mars is 21240km in diameter, elevations range over 27km. To show"
  "canyons and such, would need a very large map."
  "10m intervals for elevation would be reasonable."
  "Scenarios could include solitaire game to establish a viable settlement,"
  "and a race for Mars that is competitive."
  ""
  "(should be able to mine for water(ice) and transport to cities)"
  )))

(add spy-satellite notes (
  "Satellites are spy satellites that can see much on the ground,"
  "but require so much observing and processing time that they can"
  "only focus on small areas at a time.  So the position of a satellite"
  "designates its focus area rather than its actual physical position."
  ))
