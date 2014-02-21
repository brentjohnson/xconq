(game-module "ww2-adv"
  (title "WWII Strategic Level")
  (version "1.1")
  (blurb "Base module for strategic level WWII games. Uses armies and fleets as smallest units.")
  (picture-name "tank-column")
  (variants (world-seen true) (see-all false) (sequential false)
   ("Last Alliance Wins" last-alliance-wins true
     (true (scorekeeper (do last-alliance-wins)))))
)

;;; Types.

(unit-type infantry (image-name "inf")
  (help "a primarily foot-powered army"))
(unit-type armor (image-name "armor")
  (help "a primarily mechanized and armored army"))

(define ground-types (infantry armor))

(unit-type air-force (name "air force") (image-name "4e") (char "f")
  (help "ground support capability"))
(unit-type bomber
  (help "strategic bombing capability"))
(unit-type interceptor (image-name "1e") (char "p")
  (help "anti-bomber capability"))

(define air-types (air-force bomber interceptor))

(unit-type convoy (image-name "ap")
  (help "transportation capability"))
(unit-type fleet (image-name "bb") (char "F")
  (help "naval combat capability"))
(unit-type cv-fleet (name "carrier fleet") (image-name "cv") (char "C")
  (help "seaborne aircraft base"))
(unit-type sub-fleet (name "sub fleet") (image-name "sub")
  (help "submarine wolfpack, deadly to convoys"))
(unit-type asw-fleet (name "ASW fleet") (image-name "dd") (char "D")
  (help "submarine killers"))

(define ship-types (convoy fleet cv-fleet sub-fleet asw-fleet))

(unit-type base (image-name "airbase") (char "/")
  (help ""))
(unit-type town (image-name "town20")
  (help ""))
(unit-type city (image-name "city19")
  (help "major cities only"))
(unit-type capital (image-name "city20")
  (help "capital of a country or region"))

(define place-types (base town city capital))

(define builders         (town city capital))

(material-type oil
  (help "motive power for units"))
(material-type planes
  (help "combat power for air units"))

(terrain-type sea (char ".")
  (help "deep water"))
(terrain-type shallows (char ",")
  (help "shallow coastal water and lakes"))
(terrain-type swamp (char "="))
(terrain-type desert (char "~")
  (help "dry open terrain"))
(terrain-type land (image-name "plains") (char "+")
  (help "open flat or rolling country"))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))
(terrain-type neutral (image-name "gray") (char "-"))
(terrain-type river (char "<")
  (subtype border) (subtype-x river-x))
(terrain-type beach (char "|")
  (subtype border))
(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x))
(terrain-type mud (image-name "brown")
  (subtype coating))
(terrain-type snow (image-name "ice")
  (subtype coating))

(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (desert land forest mountains))

;;; Static relationships.

(table vanishes-on
  (ground-types sea-t* true)
  (ground-types ice true)
  (ship-types land-t* true)
  (ship-types ice true)
  (place-types sea-t* true)
  (place-types ice true)
  (u* neutral true)
  )

;;; Unit-unit capacities.

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 99)
  (ground-types (convoy fleet) 1)
  (ground-types place-types 10)
  (air-types (convoy fleet cv-fleet) 1)
  (air-types place-types 1)
  (ship-types place-types 1)
  )

(add (convoy fleet cv-fleet) capacity (2 1 2))

(add place-types capacity 40)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (u* t* 1)
  (ground-types t* 50)
  (place-types t* 100)
  )

(add t* capacity 100)

;;; Unit-material capacities.

(table unit-storage-x
  ;; This requires the units to stay close to supply centers.
  (ground-types oil (1 5))
  ;; Ships typically carry enough for six months at sea.
  (ship-types oil 30)
  (place-types oil (500 1000 2000 2000))
  (air-types planes (1000 500 1000))
  )

;;; Vision.

(set terrain-seen true)

;;; Reporters and civilians are always reporting on activities
;;; at fixed places.

(add place-types see-always true)

;; The wolfpacks and the subkillers spread out when hunting.

(add (sub-fleet asw-fleet) vision-range 2)

;; Air units all do daily patrols and thus have their eyes
;; on everything happening around them.

(add air-types vision-range 4)

;;; Actions.

(add u* acp-per-turn 1)

(add ground-types acp-per-turn (4 5))

(add air-types acp-per-turn 96)

(add ship-types acp-per-turn 48)

(add base acp-per-turn 0)

(add ground-types free-acp (1 2))

;;; Movement.

(add place-types speed 0)

(table mp-to-enter-terrain
  (ground-types sea-t* 99)
  (ground-types beach -99)
  (ground-types mud 1)
  (ground-types snow 2)
  (ship-types land-t* 99)
  ((fleet cv-fleet) shallows 2)
  (u* ice 99)
  (air-types ice 1)
  (u* neutral 99)
)

(table mp-to-leave-terrain
  (ground-types sea-t* 99)
  )

;;; Transports will get you to the edge of the cell, but the
;;; passengers have to cross any borders on their own.

(table ferry-on-entry
  (convoy ground-types over-own)
  )

(table ferry-on-departure
;  (convoy ground-types over-own)
  (convoy ground-types over-nothing)
  )

(table material-to-move
  (u* oil 1)
  )

(table consumption-per-move
  (armor oil 1)
  )

(table control-range
  (ground-types t* 0)
  (place-types t* 0)
  )

;;; Construction.

(table can-toolup-for
  (builders u* true)
  )

(table acp-to-toolup
  (builders u* 1)
  )

(table tp-to-build
  (builders infantry 6)
  (builders armor 24)
  (builders air-types 24)
  (builders ship-types 36)
  )

(table tp-max
  (builders infantry 6)
  (builders armor 24)
  (builders air-types 24)
  (builders ship-types 36)
  )

(add ground-types cp (6 9))
(add air-types cp 12)
(add ship-types cp 6)
(add base cp 1)

(table acp-to-create
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  (ground-types base 1)
  )

(table can-create
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  (ground-types base 1)
  )

(table cp-on-creation
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  (ground-types base 1)
  )

(table can-build
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

(table acp-to-build
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

(table cp-per-build
  (builders ground-types 1)
  (builders air-types 1)
  (builders ship-types 1)
  )

(table supply-on-creation
  (u* oil 1)
  (air-types planes (20 10 20))
  )

;;; Repair.

(add ground-types hp-recovery (1.00 0.50))

(table auto-repair
  (place-types ground-types 1.00)
  )

(table auto-repair-range
  ;; Replacements can easily get to nearby units.
  (u* ground-types 1)
  )

;;; Production.

(table base-production
  (place-types oil 5)
  ;; Bases are supply depots only.
  (base oil 0)
  ((city capital) planes 10)
  )

(table base-consumption
  (ground-types oil 1)
  (ship-types oil 5)
  (air-types oil (15 40 15))
  )

(table hp-per-starve
  (ground-types oil 1.00)
  (ship-types oil 1.00)
  (air-types oil 1.00)
  )

(table consumption-as-occupant
  (ground-types oil 1)
  (air-types oil 1)
  )

;;; Combat.

(add ground-types hp-max (9 6))

(add air-types hp-max 3)

(add ship-types hp-max 9)

(add place-types hp-max (3 36 48 48))

(table acp-to-attack
  (u* u* 1)
  ;; Limit the amount of combat in a month.
  (infantry u* 4)
  (armor u* 3)
  ;; Ships can always foil attacks by staying out to sea.
  (ground-types ship-types 0)
  ;; Ship combat uses up a week's worth of movement.
  (ship-types u* 12)
  ;; Convoys are not combat units.
  (convoy u* 0)
  ;; Subs are only useful against surface ships.
  (sub-fleet u* 0)
  (sub-fleet ship-types 12)
  (sub-fleet sub-fleet 0)
  ;; Airplanes don't engage in hand-to-hand.
  (air-types u* 0)
  ;; Places can't attack anything by themselves.
  (place-types u* 0)
  )

;; Air units attack by "firing" (ie raids).

(add air-types acp-to-fire 24)

(add air-types range (3 8 3))

(table withdraw-chance-per-attack
  (infantry ground-types (20 10))
  (armor    ground-types (30 20))
  )

(table acp-for-retreat
  ;; Ground units can always retreat.
  (ground-types ground-types 1)
  )

(table hit-chance
  (u* u* 50)
  (infantry ground-types (50 40))
  (armor    ground-types (70 60))
  ;; Ships are hard to shoot at from shore, but a counterattack
  ;; might connect.
  (ground-types ship-types 10)
  (fleet convoy 95)
  ;; Convoys are not combat units, but do have a couple escorts.
  (convoy u* 5)
  ;; Subs are only useful against surface ships.
  (sub-fleet u* 0)
  (sub-fleet ship-types 50)
  (sub-fleet sub-fleet 0)
  ;; Subs can generally sink *something* in a convoy.
  (sub-fleet convoy 75)
  ;; ASW is effective once it finds the subs.
  (asw-fleet sub-fleet 80)
  (place-types infantry 20)
  (place-types armor 10)
  )

(table damage
  (u* u* 1)
  ;; A fleet of surface ships can do a lot of damage.
  (fleet u* 2d3)
  ;; Subs are only useful against surface ships.
  (sub-fleet u* 0)
  (sub-fleet ship-types 1)
  (sub-fleet sub-fleet 0)
  ;; Subs can devastate a convoy.
  (sub-fleet convoy 2d4)
  ;; ASW can take a chunk out of a sub fleet.
  (asw-fleet sub-fleet 2d4+2)
  )

(table hp-min
  ;; Places can never be entirely destroyed.
  (u* place-types 1)
  ;; Naval bombardment can hurt, but not kill ground units.
  (ship-types ground-types 5)
  )

(table capture-chance
  ;; Armor can be captured and made use of.
  (ground-types armor 20)
  ;; (Ground units are not going to catch planes in the air
  ;; or ships at sea, so can only capture them if in cities.)
  ;; Cities offer basically zippo resistance to armies.
  (ground-types place-types 90)
  ;; Battle fleets can capture cargo ships and such.
  ((fleet cv-fleet asw-fleet) convoy 90)
  )

(table protection
  ;; Ground units protect cities.
  (ground-types place-types 50)
  ((air-force interceptor) place-types 80)
  (place-types ground-types 50)
  )

(table occupant-combat
  ;; Air units cannot fight while being transported on convoys.
  (air-types convoy 0)
  )

(table material-to-attack
  ;; Out-of-supply forces can defend but not initiate attacks.
  (ground-types oil 1)
  )

(table material-to-fire
  (air-types planes (100 50 100))
  )

(table consumption-per-fire
  (air-types oil 50)
  (air-types planes 10)
  )

(table hit-by
  ;; This reflect plane losses due to AA.
  (ground-types planes 1)
  (ship-types planes (1 3 4 0 2))
  (place-types planes (1 2 5 10))
  )

;;; Backdrop.

(table out-length
  ;; Net consumers of supply should never give any up automatically.
  (ground-types m* -1)
  ;; Cities and towns can share things around.
  (place-types m* 3)
  )

(table in-length
  ;; Supply to ground units can go several hexes away.
  (ground-types m* 3)
  ;; Cities and bases can get their supplies from some distance away.
  (place-types m* 10)
  (base m* 5)
  )

;;; Temperature characteristics of terrain.

(add t* temperature-min -20)
(add (sea shallows) temperature-min 4)
(add desert temperature-min 0)

(add t* temperature-max 30)
(add desert temperature-max 40)
(add mountains temperature-max 20)
(add ice temperature-max 0)

(add t* temperature-average 20)
(add mountains temperature-average 10)
(add ice temperature-average -10)

(add land temperature-variability 5)

(table coating-depth-max
  ((mud snow) t* 1)
  ((mud snow) (sea shallows neutral) 0)
  ;; Mountains are rocky, don't really have a "mud season".
  (mud mountains 0)
  )

;; If a side loses, its combat units vanish, but not its cities.

(add place-types lost-vanish-chance 0)

(set initial-date "Sep 1939")

;; Need this to be month 9.

(set initial-year-part 8)

;;; The world.

(world 360 (year-length 12))

;;; Sides.

(set sides-max 10)

(include "ng-asian")
(include "ng-european")
(include "ng-english")
(include "ng-german")
(include "ng-american")
(include "ng-chinese")
(include "ng-japanese")
(include "ng-italian")

(set side-library '(
  ((name "USA") (adjective "American") (emblem-name "flag-usa") (unit-namers (place-types "american-place-names")))
  ((name "UK") (adjective "British") (emblem-name "flag-uk") (unit-namers (place-types "english-place-names")))
  ((name "USSR") (adjective "Soviet") (emblem-name "flag-ussr") (unit-namers (place-types "russian-place-names")))
  ((name "France") (adjective "French") (emblem-name "flag-france") (unit-namers (place-types "french-place-names")))
  ((name "Germany") (adjective "German") (emblem-name "flag-swastika") (unit-namers (place-types "german-place-names")))
  ((name "Italy") (adjective "Italian") (emblem-name "flag-italy") (unit-namers (place-types "italian-place-names")))
  ((name "Japan") (adjective "Japanese") (emblem-name "flag-japan") (unit-namers (place-types "japanese-place-names")))
  ((name "China") (adjective "Chinese") (emblem-name "flag-china-old") (unit-namers (place-types "chinese-place-names")))
  ((name "Spain") (noun "Spaniard") (adjective "Spanish") (emblem-name "flag-spain") (unit-namers (place-types "spanish-place-names")))
  ((name "Turkey") (adjective "Turkish") (emblem-name "flag-turkey") (unit-namers (place-types "turkish-place-names")))
  ))

(set calendar '(usual month))

(set season-names
  ;; 0 is January, 3 is April, 6 is July, 9 is October
  ((0 2 "winter") (3 5 "spring") (6 8 "summer") (9 11 "autumn")))

;;; Random setup parameters (for testing).

(add t* alt-percentile-min   0)
(add t* alt-percentile-max   0)
(add (sea land) alt-percentile-min ( 0  70))
(add (sea land) alt-percentile-max (70 100))
(add t* wet-percentile-min   0)
(add t* wet-percentile-max 100)

;;; River generation.

(add (land forest mountains) river-chance (10.00 25.00 25.00))

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (t* land 100)
  (land (desert forest mountains) (50 40 20))
  )

(set edge-terrain ice)

(set country-radius-min 4)

(add ground-types start-with (4 1))
(add place-types start-with 3)
(add capital start-with 1)

;(add (sea land) country-terrain-min (4 4))

(table favored-terrain
  (u* t* 0)
  (ground-types land 100)
  (air-types land 100)
  (ship-types sea 100)
  (place-types land 100)
  )

(add land country-people-chance 100)

(table unit-initial-supply
  (u* oil 9999)
  (air-types planes 9999)
  )

(game-module (design-notes
  "The basic idea of this game is to model the grand strategy of the WWII era."
  "Scale is 1 deg or 90-100km/hex, 1 month/turn."
  ""
  "At this scale the details of maneuver become less important than managing"
  "production and logistics."
  ""
  "Land units are infantry and armor armies, basically the same except that armor"
  "is more highly mechanized.  Units are about corps-sized."
  ""
  "Air forces represent strategic bombing, etc abilities."
  ""
  "Convoys are primarily transport ships with some protection."
  ""
  "Fleets are battleship/cruiser fleets, while carrier fleets have a long-range"
  "strike capability due to their aircraft."
  "Submarine fleets represent a large number of subs operating over a wide area."
  ""
  "City production is very hard to change."
  ))
