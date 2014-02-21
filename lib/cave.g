(game-module "cave"
  (title "Cave of Wandering Death")
  (blurb "Explore the Cave of Wandering Death. The original cave game.")
  (picture-name "cave")
  (variants
   (world-seen false)
   (see-all false)
   (sequential false)
   (world-size)
   )
  )

(unit-type human (image-name "adventurer") (char "@")
  (hp-max 10) (acp-per-turn 4))

(unit-type orc
  (hp-max 10) (acp-per-turn 4) (cp 4))
(unit-type elf
  (hp-max 10) (acp-per-turn 4) (cp 12))
(unit-type dwarf
  (hp-max 10) (acp-per-turn 4) (cp 16))

(unit-type giant-ant (name "giant ant") (image-name "ant")
  (hp-max 15) (acp-per-turn 3) (cp 4))
(unit-type giant-beetle (name "giant beetle") (image-name "beetle")
  (hp-max 20) (acp-per-turn 4) (cp 8))
(unit-type giant-spider (name "giant spider") (image-name "spider")
  (hp-max 25) (acp-per-turn 6) (cp 12))

(unit-type white-dragon (name "white dragon") (image-name "dragon")
  (hp-max 40) (acp-per-turn 4))
(unit-type red-dragon (name "red dragon")
  (hp-max 50) (acp-per-turn 8))

(unit-type imp (image-name "person")
  (hp-max 5) (acp-per-turn 4) (cp 4))
(unit-type pit-demon (name "pit demon")
  (hp-max 50) (acp-per-turn 4) (cp 40))

(unit-type beholder
  (hp-max 50) (acp-per-turn 4) (cp 40)
  (help "a very nasty creature"))

(unit-type coins (name "pile of gold coins") (image-name "bank") (char "o"))
(unit-type sapphire (name "huge sapphire") (char "*"))
(unit-type emerald (name "huge emerald") (char "*"))
(unit-type ruby (name "huge ruby") (char "*"))
(unit-type diamond (name "huge diamond") (char "*"))

(unit-type nest
  (hp-max 50) (acp-per-turn 1))
(unit-type orc-hole (name "orc hole") (image-name "ruins")
  (hp-max 50) (acp-per-turn 1))
(unit-type pentagram (image-name "star-2")
  (hp-max 100) (acp-per-turn 1))

(define humanoids (orc elf dwarf))

(define insects (giant-ant giant-beetle giant-spider))

(define dragons (white-dragon red-dragon))

(define reptiles (white-dragon red-dragon))

(define demons (imp pit-demon))

(define other (beholder))

(define monsters (append humanoids insects reptiles demons other))

(define animate (append human monsters))

(define items (coins sapphire emerald ruby diamond))

(define places (nest orc-hole pentagram))

(material-type food)
(material-type water)
(material-type gold)

(terrain-type floor (char "."))
(terrain-type passage (image-name "flagstone") (char "."))
(terrain-type rock (char "#"))

(add rock thickness 10)

;;; Static relationships.

(add human possible-sides "human")

(add monsters possible-sides "monster")
(add places possible-sides "monster")

(include "ng-weird")

(add human namer "generic-names")
(add dragons namer "generic-names")
(add pit-demon namer "generic-names")
(add beholder namer "generic-names")

;; Unit-unit relationships.

(add human capacity 100)
(add humanoids capacity 100)
(add dragons capacity 200)

(add places capacity 8)

(table unit-size-as-occupant
  (u* u* 999)
  (items u* 1)
  (insects nest 1)
  (humanoids orc-hole 1)
  (demons pentagram 1)
  )

;; Unit-terrain relationship.

(add t* capacity (16 4 0))

(table unit-size-in-terrain
  (animate t* 4)
  (items t* 0)
  )

;; Unit-material.

(table unit-storage-x
  (human m* (200 50 0))
  )

;;; Vision.

;; (should make longer only around light sources or some such)

(add u* vision-range 5)
;; Humans are not cave dwellers by nature, don't see as well.
(add human vision-range 4)

(add items vision-range -1)

(add u* can-see-behind 0)

(table eye-height
  (u* t* 5)
  )

;;; Actions.

(add items acp-per-turn 0)

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (u* rock 99)
  )

(table mp-to-enter-own
  (u* items 0)
  )

;;; Construction.

(table can-create
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  )

(table can-build
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  )

(table acp-to-create
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  )

(table acp-to-build
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  )

;;; Combat.

(table acp-to-attack
  (u* u* 2)
  )

(table acp-to-defend
  (u* u* 1)
  )

(table hit-chance
  (u* u* 50)
  (items u* 0)
  (places u* 0)
  )

(table damage
  (u* u* 1d6)
  (u* items 0)
  (items u* 0)
  (places u* 0)
  )

(table acp-to-capture
  (u* items 1)
  )

(table capture-chance
  (u* items 100)
  )

(table independent-capture-chance
  (u* items 100)
  )

(add dragons acp-to-fire 4)
(add beholder acp-to-fire 4)

(add dragons range 6)
(add beholder range 4)

(add human cxp-max 999)

(table cxp-per-combat
  (u* u* 1)
  (u* items 0)
  )

(table damage-cxp-effect
  (human u* 1000)
  )

(table hit-cxp-effect
  (human u* 1000)
  )

;; Dwarves can dig.

(table acp-to-add-terrain
  (dwarf (floor passage) 1)
  )

(table acp-to-remove-terrain
  (dwarf rock 1)
  )

;;; Backdrop activities.

(add u* hp-recovery 1.00)

(set action-notices '(
  ((destroy u* items) (actor " smash " actee "!"))
  ((destroy u* nest) (actor " smash " actee "!"))
  ((destroy u* orc-hole) (actor " plug " actee "!"))
  ((destroy u* pentagram) (actor " erase " actee "!"))
  ))

;;; Random setup.

(add floor maze-room-occurrence 1)

(add passage maze-passage-occurrence 1)

(add t* occurrence 0)

(add rock occurrence 1)

(set edge-terrain rock)

(set maze-passage-density 5000)

;;; One adventurer on a side.

(add human start-with 1)

(add places start-with 1)
(add red-dragon start-with 1)
(add beholder start-with 1)

(set country-radius-min 5)
(set country-separation-min 20)
(set country-separation-max 30)

(table independent-density
  (items floor 200)
  )

(table favored-terrain
  (u* floor 100)
  (items passage 50)
  (u* rock 0)
  )

(set synthesis-methods
  '(make-maze-terrain make-countries make-independent-units))

(set sides-min 2)

(side 1 (name "You") (class "human") (emblem-name "none")
  (self-unit 1))

(side 2 (noun "Monster") (class "monster") (emblem-name "none"))

(scorekeeper (do last-side-wins))

(set meridians false)

(game-module (notes (
  "Cave exploration, with monsters."
  )))

(game-module (design-notes (
  "This is a pretty basic game.  It could be much elaborated."
  )))
