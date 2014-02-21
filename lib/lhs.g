(game-module "lhs"
  (title "Modern Game")
  (version "1.0")
  (blurb "The standard Xconq game, but with post-cold-war units.")
  (instructions "Take over the world before you get taken over!")
  (variants
    (world-seen false)
    (see-all false)
    (world-size (60 30 360))
    (sequential false)
    ("Mostly Land" mostly-land
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
     (true
       ;; This is the same as country separation.
       (set country-radius-max 48)
       ))
    ("Test" test
     (true
       ;; For testing.
	   (set unseen-image-name "question")
       (include "libimf")
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
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
    )
  )

(include "lhsunit")

(include "nat-names")

(include "town-names")

(include "ng-features")

(add (T @) namer "random-town-names")

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

(set scorefile-name "lhs.xcq")

(table see-others-if-captured
  (ship ship 50)
  (base u* 10)
  (town u* 20)
  (town (town city) 30)
  (city u* 100)
  )
