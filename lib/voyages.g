(game-module "voyages"
  (title "Voyages of Discovery")
  (blurb "Sail across uncharted seas, find new lands.")
  (variants
   (see-all false)
   (world-seen false)
   (sequential false)
   (world-size (80 40))
   ("Last Side Wins" last-side-wins true
     (true (scorekeeper (do last-side-wins))))
   ("Wind" wind true
     (true
       (add fleet speed-wind-effect
         ((0 ((0  10) (1 100) (3 120))) ; downwind
          (1 ((0  10) (1 120) (3 150)))
          (2 ((0  10) (1  40) (3  20)))
          (3 ((0  10) (1  10) (3  10))) ; upwind
          ))
       (add t* wind-force-min 0)
       (add t* wind-force-average 1)
       (add t* wind-force-max 3)
       (add land wind-variability 30)
       (add waters wind-variability 10)
       (set wind-mix-range 1)
       (set see-weather-always false)
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

;; should be a self-unit (?) with 2-3 hex radius of control

(unit-type explorer (image-name "conquistador")
  (help "our hero, plus bodyguards and personal servants")
  (acp-per-turn 2)
  (hp-max 3)
  )

(unit-type crew (image-name "soldiers")
  (help "supports the explorer")
  (acp-per-turn 1)
  (hp-max 30) #| (parts-max 30) |#
  )

(unit-type fleet (image-name "caravel-fleet")
  (help "transportation across the sea")
  (acp-per-turn 14)
  (hp-max 30) #| (parts-max 3) |#
  )

;;; What the crew can build to live in.  This is not a port however, ships must
;;; remain at sea.

(unit-type fort (image-name "walltown")
  (help "can be built for protection"))

(unit-type city (image-name "city18")
  (help "home port, with nearly limitless supply"))

(unit-type native-village (image-name "village")
  (help "where the more primitive natives live"))

(unit-type native-city (image-name "city18")
  (help "where the civilized natives live"))

(define native-places (native-village native-city))

(define places (fort city native-village native-city))

;; the essentials
(material-type food)
(material-type water)
(material-type wood
  (help "to build and repair ships"))
;; the goals
(material-type gold)
(material-type gems)
(material-type spices)

(define riches (gold gems spices))

;;; The usual collection of terrain types.
;;; (perhaps should flush in favor of smaller set? - no roads needed for instance)

(include "stdterr")

(define land (swamp desert plains forest mountains ice))
(define waters (sea shallows))

;;; Static relationships.

(table vanishes-on
  ((explorer crew) waters true)
)

(table wrecks-on
  (fleet land true)
)

;; A shipwreck is usable as a fort.

(add fleet wrecked-type fort)

(table unit-capacity-x
  (fleet (explorer crew) (3 3))
  (fort (explorer crew) (3 3))
  (city (explorer crew) (100 30))
  (native-village (explorer crew) 1)
  (native-city (explorer crew) (100 30))
)

(table unit-storage-x
  ;; The following need to be slightly more than production/consumption
  ;; each turn, so units don't starve immediately.
  (explorer (food water) 8)
  (crew (food water) 80)
  ;; Ships can carry 5 months of food and water for 3 crews.
  (fleet (food water) 4200)
  (fleet wood 10)
  (explorer riches 1)
  (crew riches 10)
  (fleet riches 100)
)

;;; Vision.

;; (nothing special needed?)

;;; Actions.

;;; Movement.

(table mp-to-enter-terrain
  ;; these are for accident prevention...
  ((explorer crew) waters 99)
  ;; Ship can be run aground deliberately, but only at start of turn.
  (fleet land 14)
  )

;;; Construction.

(add (fleet fort) cp (8 4))

(table can-create
  (crew (fleet fort) 1)
  )

(table can-build
  (crew (fleet fort) 1)
  )

(table acp-to-create
  (crew (fleet fort) 1)
  )

(table acp-to-build
  (crew (fleet fort) 1)
  )

;;; Combat.

(table hit-chance
  (explorer native-places 1)
  (native-places explorer (5 20))
  )

(table damage
  (u* u* 1)
  )

(table capture-chance
  (explorer places 10)
  )

;;; Production/consumption of materials.

(table base-production
  (explorer (food water) 7)
  (crew (food water) 70)
  (fleet (food water) 100)
  )

(table productivity
  ((explorer crew) t* 0)
  ((explorer crew) (plains forest swamp) 100)
  (fleet t* 10)
  )

(table productivity-adjacent
  (fleet (plains forest swamp mountains) 100)
  )

(table base-consumption
  (explorer (food water) 7)
  (crew (food water) 70)
  )

(table hp-per-starve
  ;; Going without food is bad for one's health, but not instantly fatal.
  ((explorer crew) food 1.00)
  ;; Water is essential, however.
  ((explorer crew) water 100.00)
  )

;;; Random events.

(table accident-vanish-chance
  (fleet waters (30 10))
  )

;;; Text.

(set action-notices '(
  ((destroy u* fleet) (actor " sinks " actee "!"))
  ))

(set event-notices '(
  ((unit-vanished fleet) (0 " is lost in a storm"))
  ))

(set event-narratives '(
  ((unit-vanished fleet) (0 " was lost in a storm"))
  ))

;;; Starting material for random setups.

(define country-radius-min 2)

(add (sea plains) country-terrain-min (1 3))

(add (explorer crew fleet city) start-with (1 1 1 1))

(table favored-terrain
  (u* t* 0)
  ((explorer crew) plains 100) 
  (city plains 100)
  (fleet sea 100)
  ((native-village native-city) land 100)
  )

(add city initial-seen-radius 6)

(table independent-density
  (native-village (plains forest mountains) (100 50 20))
  (native-city plains 10)
  )

(include "ng-weird")

(add native-places namer "generic-names")

(table unit-initial-supply
  ;; Everybody starts out with plenty of food and water.
  (u* (food water) 9999)
  (native-city gold 100)
  )

;;; This works as a solo game, but more can play if desired.

(set sides-min 1)
(set sides-wanted 5)
(set sides-max 5)

;;; Players can give them more explorers and ships to start with.

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 5)

(include "ng-european")
(include "ng-english")

(set side-library '(
  ((noun "Spaniard") (emblem-name "flag-spain-old") (unit-namers (places "spanish-place-names")))
  ((noun "Portuguese") (emblem-name "flag-portugal") (unit-namers (places "portuguese-place-names")))
  ((noun "English") (emblem-name "flag-uk") (unit-namers (places "english-place-names")))
  ((noun "French") (emblem-name "flag-france") (unit-namers (places "french-place-names")))
  ((noun "Dutch") (emblem-name "flag-dutch") (unit-namers (places "dutch-place-names")))
  ))

;;; should always generate a globe-girdling area to play on.

(game-module
  (notes
	"might want a general operates-equipment relation so crew can move via
	ships but can still leave, thus immobilizing the ships"
	"explorer would have to manage crews properly"

	"crews on land can collect water/food and store on ship"

	"forts protect crew while they forage(?)"

	"high chance of attrition for crew"
	"Explorer only vulnerable to combat or starvation however"

	"need to do storms somehow (stripped-down lat-based weather model?)"

  "Turn length is about a week."
 ))
