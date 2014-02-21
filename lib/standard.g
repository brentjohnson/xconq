(game-module "standard"
  (title "Default Game")
  (version "7.3+")
  (blurb "The default Xconq game, loosely based on WW II ca 1945.")
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
    ("Ice-Age" ice-age
      "Generate a world that has normal amounts of water but almost all land is ice."
      (true
        (add swamp alt-percentile-min 69)
        (add swamp alt-percentile-max 70)
        (add (desert plains forest) alt-percentile-max 74)
        (add mountains alt-percentile-min 74)
        (add mountains alt-percentile-max 78)
        (add ice alt-percentile-min 78)
        (add ice alt-percentile-max 100)
	(set edge-terrain swamp)

        ;; Try to make it so there is room for countries.  It still is harder
	;; to meet the constraints in a many-player game than the non-iceage
	;; variant, but there may be only so much we can/should do.
        (set country-radius-min 1)
        (add (sea plains) country-terrain-min (1 3))
        (table favored-terrain add (@ plains 20))
        ;; To get a somewhat similar number of towns per land, need more
        ;; towns per plains/desert/forest/mountains.
	(table independent-density
	  (town plains 3000)
	  (town (desert forest mountains) 500))
	;; In keeping with the ice-age theme change desert to tundra
	;; (with no change in properties).
	(imf "civ-tundra" ((1 1) 214 255 231))
	(imf "civ-tundra" ((44 48) (x 2 46 0) (file "civt44x48.gif" 186 152)))
	(add desert image-name "civ-tundra")
	(add desert help "Desert (tundra) is fairly flat but less hospitable than plains")

	;; Now pick arctic country names.
	(set side-library '(
	  (50 (noun "Norwegian") (emblem-name "flag-norway")
	    (unit-namers ((town city) norwegian-place-names)))
	  (50 (noun "Swede") (adjective "Swedish") (emblem-name "flag-sweden")
	    (unit-namers ((town city) swedish-place-names)))
	  (50 (noun "Finn") (adjective "Finnish") (emblem-name "flag-finland")
	    (unit-namers ((town city) finnish-place-names)))
	  (50 (noun "Icelander") (adjective "Icelandic") (emblem-name "flag-iceland")
	    (unit-namers ((town city) icelandic-place-names)))
	  (50 (noun "Canadian") (emblem-name "flag-canada")
	    (unit-namers ((town city) canadian-place-names)))
	  (50 (noun "Russian") (emblem-name "flag-russia")
	    (unit-namers ((town city) russian-place-names)))
	  (50 (noun "Mongol") (adjective "Mongolian")
	    (emblem-name "flag-mongolia")
	    (unit-namers ((town city) mongolian-place-names)))
	  (50 (noun "Nepalese") (emblem-name "flag-nepal")
	    (unit-namers ((town city) nepalese-place-names)))
	  (50 (noun "Afghan") (emblem-name "flag-afghanistan")
	    (unit-namers ((town city) afghan-place-names)))
	  (50 (noun "Chilean") (emblem-name "flag-chile")
	    (unit-namers ((town city) chilean-place-names)))

	  ;; Not sure this image really works for a flag.
	  ;; (50 (noun "Viking") (emblem-name "heroes-viking") ...)
	  ;; (imf "heroes-viking" ((44 44) (file "heroes.gif" 140 354)))
	  ;; Also would like: Tibet, Inuits, Aleutians
	  ))
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

    ("Classic" classic
     "Use pre-7.5 graphics." 
     (true
       ;; No capability for alternate image sets, so do it the hard way.
       (add infantry image-name "soldiers")
       (add armor image-name "tank")
       (add fighter image-name "fighter")
       (add bomber image-name "4e")
       (add destroyer image-name "dd")
       (add submarine image-name "sub")
       (add troop-transport image-name "ap")
       (add carrier image-name "cv")
       (add battleship image-name "bb")
       (add base image-name "airbase")
       (add town image-name "town20")
       (add city image-name "city20")
       ))

    ("Alternate Backdrop Economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))

    ("Alternate Backdrop Econ. uses Doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))

    ("More Towns" density1 
     "Five times as many towns."
     true
     (true (table independent-density (town plains 500)))
    )

    )
  )

(include "stdunit")

(include "nat-names")

(include "town-names")

(include "ng-features")

(add B namer "junky")

(add (T @) namer "random-town-names")

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
(set advantage-max 15)

(scorekeeper
  (title "")
  (do last-side-wins)
  )

(set scorefile-name "standard.xcq")

;; Leave see-terrain-if-captured to zero.  This is so that exploration
;; does not cease early in the game.  Alternatives might be to 
;; make all hexes within a fixed or randomly determined radius
;; from the captured unit visible, or even a make-terrain like
;; algorithm that generates one single random sized area of visible hexes
;; that nucleates at the captured unit.  Some terrain info will leak
;; out via see-others-if-captured - that's a more appropriate quantity
;; than the all-or-nothing see-terrain-if-captured.

(table see-others-if-captured
  (ship ship 50)
  (base u* 10)
  (town u* 15)
  (city u* 100)
  )

(add places already-seen 100)
(add places already-seen-independent 100)
