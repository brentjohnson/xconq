(game-module "quest"
  (title "Quest for Adventure")
  (blurb "Wilderness exploration in a fantasy world.")
  (variants (world-seen false) (see-all false)(sequential false))
  (instructions (
    "Explore the world, looking for treasure and killing monsters."
  ))
  )

(unit-type novice (image-name "adventurer") (char "@")
  (hp-max 10)
  )
(unit-type wanderer (image-name "adventurer") (char "@")
  (hp-max 20)
  )
(unit-type adventurer (char "@")
  (hp-max 40)
  )
(unit-type master (image-name "adventurer") (char "@")
  (hp-max 80)
  )

;; should add other levels of human

(unit-type goblin (image-name "orc")
  (hp-max 4)
  )
(unit-type skeleton
  (hp-max 5)
  )
(unit-type orc
  (hp-max 10)
  )
(unit-type elf
  (hp-max 15)
  )
(unit-type dwarf
  (hp-max 9)
  )
(unit-type bugbear
  (hp-max 10)
  )
(unit-type troll
  (hp-max 20)
  )
(unit-type centaur (image-name "horse")
  (hp-max 20)
  )
(unit-type yeti
  (hp-max 20)
  )
(unit-type dragon
  (hp-max 40)
  )

(unit-type ant
  (hp-max 5)
  )
(unit-type beetle
  (hp-max 20)
  )
(unit-type spider
  (hp-max 20)
  )
(unit-type scorpion
  (hp-max 20)
  )

(unit-type vampire
  (hp-max 40)
  )

(unit-type sorceror
  (hp-max 40)
  )

(unit-type purple-worm
  (hp-max 40)
  )

(unit-type beholder
  (hp-max 40)
  )

(define human (novice wanderer adventurer master))

(define humanoid (goblin orc elf dwarf bugbear yeti sorceror))

(define reptile (dragon))

(define bug (ant beetle spider scorpion))

(define undead (skeleton vampire))

(define other (troll centaur purple-worm beholder))

(define monster (append humanoid reptile bug undead other))

(define animate (append human monster))

(define living (append human humanoid reptile bug centaur purple-worm beholder))

(unit-type diamond-ring (image-name "ring")
  )
(unit-type treasure-chest
  )

(define objects (diamond-ring treasure-chest))

(unit-type lair (image-name "ruins")
  (hp-max 20)
  )
(unit-type village
  (hp-max 20)
  )
(unit-type town (image-name "town20") (char "*")
  (hp-max 50)
  )
(unit-type castle (char "K")
  (hp-max 50)
  )
(unit-type city (image-name "city18") (char "!")
  (hp-max 100)
  )

(define places (lair village town castle city))

(material-type food
  )
(material-type water
  )
(material-type gold
  )

(include "stdterr")

(define water-t* (sea shallows))
(define land (swamp plains forest desert mountains))

(include "ng-weird")

(add human namer "generic-names")

(add places namer "generic-names")
(add lair namer "")

(add human possible-sides "human")

(add monster possible-sides "monster")

(add lair possible-sides "monster")

;;; Static relationships.

(table vanishes-on
  (animate water-t* true)
  (dragon water-t* false)
  (yeti t* true)
  (yeti (mountains ice) false)
  (places water-t* true)
  (places ice true)
  )

;;; Unit-unit capacities.

(add human capacity 10)

(add places capacity (20 20 40 40 80))

(table unit-size-as-occupant
  (u* u* 100)
  (animate places 1)
  ;; Rings are so small anybody can can carry hundreds.
  (diamond-ring human 0)
  (treasure-chest human 5)
  )

;;; Unit-terrain capacities.

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (village t* 9)
  (town t* 12)
  (castle t* 9)
  (city t* 16)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (living food 1)
  (living water 4)
  (novice m* (10 5 100))
  (wanderer m* (10 5 100))
  (adventurer m* (10 5 100))
  (master m* (10 5 100))
  (dragon gold 1000)
  (treasure-chest gold 1000)
  )

;;; Vision.

(add places already-seen 100)
(add lair already-seen 0)
(add village already-seen 50)
(add castle already-seen 50)

;;; Actions.

(add u* acp-per-turn 4)
(add places acp-per-turn 0)
(add lair acp-per-turn 1)

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  ;; Accident prevention.
  (animate water-t* 99)
  (animate mountains 2)
  (animate ice 3)
  (animate river 0)
  (humanoid river 1)
  (animate road 0)
  (dwarf mountains 1)
  ;; Insects aren't intelligent enough to figure out how to cross water.
  (bug river 99)
  (yeti t* 99)
  (yeti (mountains ice) 0)
  ;; Dragons can go anywhere.
  (dragon t* 0)
  )

(table mp-to-leave-terrain
  ;; Forests are hard to get out of.
  (humanoid forest 1)
  ;; ...but not for elves.
  (elf forest 0)
  ;; Swamps are hard to get out of too.
  (humanoid swamp 1)
  (elf swamp 0)
  ;; Note that we make mountains harder to enter,
  ;; but exact no penalty for departure (walking
  ;; downhill is easy).
  )

(table mp-to-traverse
  (animate road 1)
  )

(table can-enter-independent
  (human places true)
  ((elf dwarf) places true)
  )

;;; Construction.

(add monster cp 5)

(table can-create
  (lair monster 1)
  )

(table can-build
  (lair monster 1)
  )

(table acp-to-create
  (lair monster 1)
  )

(table acp-to-build
  (lair monster 1)
  )

;;; Combat.

(table acp-to-attack
  (animate places 0)
  (dragon places 2)
  )

(table hit-chance
  (u* u* 50)
  (animate places 0)
  (dragon places 100)
  (beholder u* 80)
  (purple-worm u* 80)
  (places u* 0)
  )

(table damage
  (u* u* 1)
  (animate places 0)
  (beholder u* 1d4)
  (dragon animate 1d5)
  (dragon places 1d4)
  (purple-worm animate 1d10)
  (places u* 0)
  )

;;; Backdrop.

(add u* hp-recovery 100)

(table base-production
  (living food 1)
  (living water 4)
  (places food 10)
  (places water 100)
  )

(table productivity
  (living desert 0)
  ;; Insect types are usually well-adapted to desert.
  (bug desert 100)
  (lair land (0 100 50 20 20))
  (village land (0 100 50 20 20))
  (town land (0 100 50 20 20))
  (city land (0 100 50 20 20))
  )

(table base-consumption
  (living food 1)
  (living water 1)
  )

(table hp-per-starve
  (living food 1.00)
  ;; Dragons are very tough.
  (dragon food 0.10)
  (living water 1.00)
  (dragon water 0.10)
  )

;;; Random setup.

(add cell-t* alt-percentile-min (  0  20  24  24  24  24  90  97))
(add cell-t* alt-percentile-max ( 20  24  30  90  90  90  97 100))
(add cell-t* wet-percentile-min (  0   0  50   0  20  80   0   0))
(add cell-t* wet-percentile-max (100 100 100  20  80 100 100 100))

;;; One adventurer on a side.

(add novice start-with 1)

;; A sampling of monsters.
(add monster start-with 1)
(add orc start-with 5)
(add lair start-with 3)

(set country-radius-min 8)
(set country-separation-min 15)
(set country-separation-max 20)

(table favored-terrain
  (u* t* 0)
  (u* (sea shallows) 0) 
  (u* plains 100)
  (elf forest 100)
  (dwarf plains 20)
  (dwarf mountains 100)
  (centaur forest 100)
  (scorpion desert 100)
  (yeti t* 0)
  (yeti mountains 50)
  (yeti ice 100)
  (dragon mountains 100)
  (lair plains 10)
  (lair (desert forest mountains) (20 100 100))
  )

(table independent-density
  (village (plains forest) (300 100))
  (town (plains forest) (100 50))
  (castle (plains forest mountains) (100 50 50))
  (city plains 10)
  (objects (plains forest) 20)
  )

(table road-chance
  (town (town city) ( 2   5))
  (castle (town city) 20)
  (city (town city) (80 100))
  )

(add (town city) road-to-edge-chance 100)

(set edge-road-density 100)

;; Nearly all towns and villages should be connected by road to
;; somewhere else.

(add village spur-chance 50)
(add village spur-range 2)

(add town spur-chance 90)
(add town spur-range 2)

(add castle spur-chance 90)
(add castle spur-range 2)

(set sides-min 2)
(set sides-max 2)

(side 1 (name "You") (class "human"))

(side 2 (noun "Monster") (class "monster") (emblem-name "none"))

(table unit-initial-supply
  ;; Give everybody a full supply of food and water initially.
  (u* food 10000)
  (u* water 10000)
  (u* gold 0)
  )

(scorekeeper (do last-side-wins))

(include "ng-features")

(set feature-types '(continents islands seas lakes bays
			    (desert 10)(forest 10)(mountains 5) peaks))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))

(game-module (notes (
  "This is an outdoors adventure for individuals."
  )))

;;; Various pieces of equipment should be available.

;;; Have nobrains machine-run sides for countries, towns, castles, etc.
;;; (or quiescent unless player becomes outlaw?)

;;; Monsters belong to sides defined by alignment.  Sides of matching
;;; alignment friendly, etc.  No brains to side, but player could run
;;; a monster side, just for fun.
;;; Monsters can breed periodically, but limit total # somehow.

;;; Win by collecting the most treasure after <n> turns.
;;; Treasure should be hidden inside towns, carried by monsters.
