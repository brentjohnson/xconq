;; Looking for a better name than "logistics game"!  Send your ideas
;; to the mailing list.

(game-module "log"
  (title "Logistics Game")
  (version "7.3+")
  (blurb "Like the standard Xconq game, but you need to establish supply lines.")
  (instructions "Take over the world before you get taken over!")
  (variants
    (world-seen false)
    (see-all false)
    (world-size (60 30 360))
    (sequential false)
    ("Mostly Land" mostly-land
      "Generate a world that is mostly (80%) land."
      (true
        ;; Adjust so that sea is 20% instead of 70% of the world.
        (add sea alt-percentile-max 20)
        (add shallows alt-percentile-min 20)
        (add shallows alt-percentile-max 21)
        (add swamp alt-percentile-min 21)
        (add swamp alt-percentile-max 23)
        (add (desert plains forest) alt-percentile-min 21)
        ))
    ("All Land" all-land
      "Generate a world that is all different types of land."
      (true
        ;; Adjust sea and shallows out of existence, let swamp take all the low spots.
        (add sea alt-percentile-min 0)
        (add sea alt-percentile-max 0)
        (add shallows alt-percentile-min 0)
        (add shallows alt-percentile-max 0)
        (add swamp alt-percentile-min 0)
        (add swamp alt-percentile-max 2)
        (add swamp wet-percentile-min 0)
        (add swamp wet-percentile-max 100)
        (add (desert plains forest) alt-percentile-min 2)
        ;; Counterproductive to try to set up near water.
		(add sea country-terrain-min 0)
        ))
    ("Large Countries" large
     "Grow each starting country to take as much land as possible."
     (true
       ;; This is the same as country separation.
       (set country-radius-max 48)
       (add (town city) unit-growth-chance (100 20))
       (add (town city) independent-growth-chance (20 0))
       ))
    ("Noisy" noisy
     (true
       (set action-movies '(
         (move (sound "pop"))
         ))

       (set event-movies '(
         (side-lost (sound "thunder"))
         (unit-captured (sound "chirr"))
         (unit-completed (sound "chirr"))
         (unit-moved (sound "pop-2"))
         ))
       ))
    ("Silhouettes" silhouettes
     "Use silhouettes for unit display instead of color images."
     (true
       ;; No capability for alternate image sets, so do it the hard way.
       (add infantry image-name "soldiers-s")
       (add armor image-name "tank-s")
       (add fighter image-name "fighter-s")
       (add bomber image-name "4e-s")
       (add destroyer image-name "dd-s")
       (add submarine image-name "sub-s")
       (add troop-transport image-name "ap-s")
       (add carrier image-name "cv-s")
       (add battleship image-name "bb-s")
       (add nuclear-bomb image-name "bomb-s")
       (add base image-name "airbase-s")
       (add town image-name "town20-s")
       (add city image-name "city20-s")
       ))
    )
  )

(include "logunit")

(include "nat-names")

(include "town-names")

(include "ng-features")

(add B namer "junky")

(add (~ @) namer "random-town-names")

(set default-namer "random-town-names")

(set feature-types '(continents islands seas lakes bays
			    (desert 10)(forest 10)(mountains 5) peaks))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 5)

(scorekeeper
  (title "")
  (do last-side-wins)
  )

(table see-others-if-captured
  (ship ship 50)
  (base u* 10)
  (town u* 20)
  (town (town city) 30)
  (city u* 100)
  )
