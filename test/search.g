;;; Test case for searching/exploration.

(game-module "t-search"
  (base-module "intro")
  (variants ("More Land" more-land (true
    (add t* alt-percentile-min (0 10 90))
    (add t* alt-percentile-max (10 90 100))
    ))
   )
  )

(add u* start-with 0)
(add armor start-with 4)
(add u* independent-near-start 0)

(set country-separation-min 30)
(set country-separation-max 40)

