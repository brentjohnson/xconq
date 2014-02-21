(game-module "modern"
  (title "Modern Times")
  (blurb "Economics and politics of today's world")
  (variants (see-all true))
  )

;;; To do list:
;; * Need military units and combat
;; * Need to finish the job on bases and ports (currently no way to construct)
;; * Need some way of producing the "people" material - and something that
;; happens if you don't have enough.  Either that or get rid of it.
;; * Need to be able to move supplies around somehow.  Cargo-ships
;; have neither in-length or out-length; oil-tankers have only an in-length.
;; There is no land unit which either can actually move with the supplies
;; or which has in-length/out-length (the latter being similar to the
;; warehouse in empire.g, and more in keeping with the xconq philosophy
;; of keeping material distribution automatic).
;; * I assume we want cities to set hp-recovery?
;; * Building ships - (1) shipyard needs acp-per-turn, (2) can a ship
;; be an occupant of a shipyard?  What happens now is that the ship
;; gets put in the terrain of the shipyard, and then vanishes, (3)
;; how many construction points for ships?

;; This could be a variant, but is implausible to have an unseen modern world.
(set terrain-seen true)

(unit-type engrs (image-name "engr")
  (help "builders of everything"))

(unit-type fishing-fleet (image-name "ap")
  (help "ships that produce food"))
(unit-type cargo-ship (image-name "ap")
  (help "ships that carry materials around"))
(unit-type oil-tanker (image-name "ap")
  (help "ships that carry oil"))

(define ship-types (fishing-fleet cargo-ship oil-tanker))

(unit-type farm (image-name "barn")
  (help "the main producer of food"))
(unit-type coal-mine (image-name "mine-entrance"))
(unit-type oil-field (image-name "oil-derrick")
  (help "land-based producer of crude oil"))
(unit-type oil-platform (image-name "oil-derrick")
  (help "sea-based producer of crude oil, very expensive"))
(unit-type sawmill (image-name "facility"))
(unit-type iron-mine (image-name "iron-mine"))

(define extraction-types
  (farm oil-field oil-platform sawmill iron-mine coal-mine))

(unit-type refinery (image-name "refinery"))
(unit-type steel-mill (image-name "facility"))

(define processing-types (refinery steel-mill))

(unit-type shipyard (image-name "facility"))

(define manufacturing-types (shipyard))

(unit-type base (image-name "airbase"))
(unit-type port (image-name "anchor"))

(define base-types (base port))

(unit-type town (image-name "town20"))
(unit-type city (image-name "city20"))

(define cities (town city))

(define place-types
  (append extraction-types processing-types manufacturing-types base-types cities))

(material-type food
  (help "all units consume this"))
(material-type coal
  (help "rocks that burn"))
(material-type oil
  (help "crude oil from the ground"))
(material-type petrol
  (help "refined petroleum products"))
(material-type lumber
  (help "wooden construction materials"))
(material-type iron-ore
  (help "iron-bearing rock from the ground"))
(material-type steel
  (help "represents refined metal construction materials"))
(material-type people
  (help "represents about 1,000 actual persons"))

(include "stdterr")

(define water-t* (sea shallows river))

;;; Static relationships.

(table vanishes-on
  ;; Fixed places are almost always land-only.
  (place-types water-t* true)
  (place-types ice true)
  ;; Offshore oil platforms are sea-only.
  (oil-platform t* true)
  (oil-platform (sea shallows) false)
  ;; It's possible, if not easy, to build an airbase on an icefield.
  (base ice false)
  (engrs water-t* true)
  )

(table unit-capacity-x
  ;; Engineers can always be in any kind of facility.
  (place-types engrs 2)
  (cities engrs 16)
  )

(table unit-storage-x
  ;; Cities and towns always have some room for everything.
  (city m* 100)
  (town m* 10)
  ;; Assume the food pipeline to everybody has about a month's
  ;; supply in it somewhere.
  (u* food 4)
  (farm food 100)
  (cities food 900)
  (cities oil 400)
  (u* petrol 10)
  (cities petrol 400)
  (engrs lumber 10)
  (engrs steel 10)
  (shipyard steel 100)
  (fishing-fleet food 50)
  (cargo-ship lumber 100)
  (cargo-ship steel 100)
  (oil-tanker oil 1000)
  ;; Most units involve about 1,000 people, but towns and cities
  ;; may get much larger.
  (u* people 1)
  (town people 200)
  (city people 20000)
  ;; Let processing-types store 2 turns worth of inputs.
  (refinery oil 200)
  (steel-mill coal 200)
  (steel-mill iron-ore 200)
  )

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  ;; We can pack multiple facilities into a cell.
  (farm t* 4)
  (oil-field t* 4)
  (refinery t* 4)
  (town t* 4)
  ;; A major city is so large that it fills the cell.
  (city t* 16)
  )

(table terrain-storage-x
  (t* oil 100)
  (t* iron-ore 100)
  )

;;; Actions.

(add engrs acp-per-turn 1)
(add cities acp-per-turn 1)
(add processing-types acp-per-turn 1)

;;; Movement.

(add place-types speed 0)

;;; Construction.

(add engrs cp 2)
(add extraction-types cp 24)
(add oil-platform cp 144)
(add processing-types cp 48)
(add manufacturing-types cp 96)

(table can-create
  (engrs extraction-types 1)
  (engrs processing-types 1)
  (engrs manufacturing-types 1)
  (shipyard ship-types 1)
  (cities engrs 1)
  )

(table acp-to-create
  (engrs extraction-types 1)
  (engrs processing-types 1)
  (engrs manufacturing-types 1)
  (shipyard ship-types 1)
  (cities engrs 1)
  )

(table cp-on-creation
  (engrs (farm oil-field oil-platform shipyard) 3)
  )

(table create-range
  (engrs oil-platform 1)
  (city oil-field 2)
  )

(table can-build
  (engrs extraction-types 1)
  (engrs processing-types 1)
  (engrs manufacturing-types 1)
  (shipyard ship-types 1)
  (cities engrs 1)
  )

(table acp-to-build
  (engrs extraction-types 1)
  (engrs processing-types 1)
  (engrs manufacturing-types 1)
  (shipyard ship-types 1)
  (cities engrs 1)
  )

(table cp-per-build
  (engrs (farm oil-field oil-platform shipyard) 3)
  )

(table build-range
  (city oil-field 2)
  )

(table consumption-per-built
  ;; Every kind of construction needs a small amount of lumber.
  (extraction-types lumber 2)
  (processing-types lumber 2)
  (ship-types steel 1)
  (extraction-types steel 2)
  (processing-types steel 2)
  )

;;; Combat.

(add cities hp-max (5 25))

;;; Disbanding.

;; If the player makes a mistake and is using up supplies with a
;; unit they don't need, offer them a way out.  Might as well make
;; it simple, rather than lengthy, I guess.
(add u* acp-to-disband 1)

;;; Terrain alteration.

(table acp-to-add-terrain
  (engrs road 1)
  )

(table acp-to-remove-terrain
  (engrs road 1)
  )

;;; Backdrop.

;; Automatic production.

(table base-production
  ((fishing-fleet farm) food (50 50))
  (coal-mine coal 100)
  (sawmill lumber 10)
  ((oil-field oil-platform city) oil (100 50 20))
  (city petrol 20)
  (iron-mine iron-ore 100)
  )

(table productivity
  ;; Farms are less efficient in hostile terrain.
  (farm (desert mountains) 10)
  (farm forest 50)
  ;; Sawmills can only operate at full capacity in forests and mountains.
  (sawmill t* 0)
  (sawmill (forest mountains) 100)
  ;; Plains will have small patches of woods to exploit.
  (sawmill plains 10)
  ((iron-mine coal-mine) t* 10)
  ((iron-mine coal-mine) mountains 100)
  )

(table base-consumption
  (u* food 1)
  ((town city) food (2 10))
  (city oil 10)
  ;; Everything in the modern world runs on petroleum products.
  (u* petrol 1)
  ;; production-types convert inputs (e.g. oil) into outputs (e.g. petrol).
  ;; We produce the outputs via explicit production and consume the inputs
  ;; via base-consumption.  This is somewhat imperfect (the base-consumption
  ;; happens even if the player tries to skip a few turns, so material
  ;; vanishes without any warning).  However, it does have the desired
  ;; feature of production stopping if the input is not available.
  ;; That's why hp-per-starve is not set for production-types
  ((town city) petrol (2 10))
  (refinery oil 100)
  ;; Strictly speaking, steel mills use coke, but let's assume they
  ;; always make their own coke.
  (steel-mill coal 100)
  (steel-mill iron-ore 100)
  ;; Towns and cities need some steel for repairs and renovation.
  (cities steel (1 5))
  )

(table hp-per-starve
  ;; Excluding farm from food starvation is to get rid of an annoying
  ;; case in which you build a farm (while incomplete it has no base
  ;; consumption), next turn it will have plenty of food (because it
  ;; produces it itself), but this turn it is all out and it starves.
  (u* food 1.00)
  (farm food 0.0)
  (u* petrol 0.02)
  (refinery petrol 0.0)
  ;; Without repair materials, things slowly disintegrate.
  (cities steel (0.02 0.10))
  )

(table out-length
  (u* food 4)
  (coal-mine coal 4)
  (iron-mine iron-ore 4)
  (cities coal 4)
  ((oil-field oil-platform) oil 6)
  (cities oil 6)
  (refinery petrol 6)
  (cities petrol 6)
  (sawmill lumber 6)
  (steel-mill steel 6)
  )

(table in-length
  (u* food 4)
  (steel-mill coal 4)
  (steel-mill iron-ore 4)
  (cities coal 4)
  (oil-tanker oil 6)
  (refinery oil 6)
  (cities oil 6)
  (u* petrol 6)
  (engrs lumber 6)
  (engrs steel 6)
  )

(table acp-to-produce
  (refinery petrol 1)
  (steel-mill steel 1)
  )

(table material-per-production
  (refinery petrol 100)
  (steel-mill steel 100)
  )

;; This is checked after base-consumption each turn
;; so it probably could be even lower (?)
(table material-to-produce
  (refinery oil 50)
  (steel-mill coal 50)
  (steel-mill iron-ore 50)
  )

;;; Initial random setup.

(add city start-with 1)
(add town start-with 5)
(add farm start-with 10)

(set country-radius-min 3)
(set country-separation-min 16)
(set country-separation-max 48)
; Try to get countries on the coast.
(add (sea plains) country-terrain-min (1 4))

(table favored-terrain
  (u* t* 0)
  (farm plains 100)
  (cities plains 100)
  (cities land-t* 20)
  (cities plains 40)
  )

(table independent-density (cities plains 100))

(add land-t* country-people-chance 90)
(add plains country-people-chance 100)

(add land-t* independent-people-chance 50)

(table unit-initial-supply
  (u* m* 9999)
  (cities petrol 400)
  (city people 2000)
  )

(table terrain-initial-supply
  (t* oil 7d15)
  )

;; Doctrine.
;; Building extraction-types, processing-types, manufacturing-types,
;; or ship-types, should definitely be a run of 1, because they consume
;; scarce materials to build, and take a long time.
;; As for cities building engrs, well, 1 works too because you don't
;; want too many engrs sucking up petrol.  But that one is more debatable.
(doctrine default-doctrine
  (construction-run (u* 1))
  )

(side-defaults
  (default-doctrine default-doctrine)
  )

;; Allow from 1 to many sides to play.

(set sides-min 1)

(game-module (design-notes (
  "A detailed game of modern problems."
  "Scale is 1 week/turn, 50km/cell."
  ""
  "Although this is a lot like the net empire games, it does not
   need to emulate them."
  "(should have a many-nukes-available option)"
  )))
