(game-module "monster"
  (title "Monster")
  (version "1.0")
  (blurb "Monsters stomping on cities.")
  (variants (world-seen true)(see-all true)(sequential false)
     ("Last Side Wins" last-side-wins true
     (true (scorekeeper (do last-side-wins))))
  )  
)

;;; Reliving those old movies...

; (set see-all true)

(unit-type monster (image-name "monster") (char "M")
  (point-value 100)
  (help "breathes fire and stomps on buildings"))
(unit-type mob (name "panic-stricken mob") (image-name "horde") (char "m")
  (help "helpless civilians"))
(unit-type |fire department| (image-name "firetruck") (char "f")
  (help "puts out fires"))
(unit-type |national guard| (image-name "soldiers") (char "g")
  (help "does battle with the monster"))
(unit-type building (image-name "city20-intact") (char "b")
  (point-value 1)
  (help "good for hiding, but crushed by monster"))
(unit-type |burning building| (image-name "city20-burning") (char "B")
  (point-value 1)
  (help ""))
(unit-type |wrecked building| (image-name "city20-wrecked") (char "W")
  (point-value 0)
  (help ""))
(unit-type |rubble pile| (image-name "city20-rubble") (char "r")
  (point-value 0)
  (help ""))

(define firedept |fire department|)
(define guard |national guard|)
(define BB |burning building|)
(define WB |wrecked building|)
(define R |rubble pile|)

(terrain-type sea (char "."))
(terrain-type beach (image-name "desert") (char ","))
(terrain-type street (image-name "light-gray") (char "+"))
(terrain-type block (image-name "tan") (char "-"))
(terrain-type fields (image-name "plains") (char "="))
(terrain-type trees (image-name "forest") (char "%"))

(define movers (monster mob firedept guard))
(define water (sea))
(define land (beach street block fields trees))

(add sea liquid true)

; Using undefined sides here causes access violations under Windows
; but works fine in the Unix and Mac interfaces. Why?
;(add (monster) possible-sides "monster")
;(add (mob firedept guard building) possible-sides "human")

;;; Static relationships.

(table unit-capacity-x
  ((building BB WB R) guard 1)
  ;; They can start in a building, but cannot re-enter.
  ((mob firedept) building 1)
  )

(table vanishes-on
  (u* water true)
  ;; Godzilla can go in the water.
  (monster water false)
  ;; Fire trucks and mobs can only go along the streets.
  ((mob firedept) t* true)
  ((mob firedept) street false)
  )

;;; Actions.

(add movers acp-per-turn (2 1 2 2))

(table mp-to-enter-terrain
  (u* water 99)
  (monster water 0)
  ;; Fire trucks and mobs can only go along the streets.
  ((mob firedept) t* 99)
  ((mob firedept) street 0)
  )

(table mp-to-enter-unit
  (u* u* 99)
  ;; The national guard can go through city blocks to get somewhere.
  (guard (building WB R) 1)
  )

(table can-repair
  (firedept BB true)
  )

(table acp-to-repair
  (firedept BB 1)
  )

(table hp-per-repair
  (firedept BB 1.50)
  )

;;; Combat.

;;                M m f g b B W r
(add u* hp-max (100 1 2 5 6 6 6 1))

(table hit-chance
  (u* u* 0)
  ;;              M   m   f   g   b   B   W   r
  (monster  u* ( 50  50  50  50 100  90  80   0))
  (firedept u* (  0   0   0   0   0  50  30   0))
  (guard    u* ( 80   0   0   0  50  30  30   0))
  (building u* ( 10   0   0   0   0   0   0   0))
  )

(table damage
  (u* u* 1)
  ;; Monster takes longer to damage already-damaged buildings.
  (monster building 3)
  (monster BB 2)
  (monster WB 1)
  (firedept (building BB WB) 2)
  (guard monster 4)
  ;; Rubble piles are pretty much indestructible.
  (u* |rubble pile| 0)
  )

(table withdraw-chance-per-attack
  ;; The monster has no real desire to fight.
  (monster guard 40)
  ;; The people are brave, but they're not stupid.
  ((guard firedept) monster (20 40))
  )

(add monster acp-to-fire 1)

(add monster range 2)

(add building wrecked-type BB)

(add BB wrecked-type WB)

(add WB wrecked-type R)

(add BB hp-per-detonation 4)

(table detonation-unit-range
  (BB u* 2)
  )

(table detonation-damage-at
  (BB u* 6)
  (BB R 0)
  )

(table detonation-damage-adjacent
  (BB u* 6)
  (BB R 0)
  )

(table detonation-accident-chance
  (BB t* 10.00)
  )

(set action-notices '(
  ((destroy u* monster) (actor " kills " actee "!"))
  ((destroy u* mob) (actor " massacres " actee "!"))
  ((destroy u* building) (actor " sets " actee " on fire!"))
  ((destroy u* BB) (actor " wrecks " actee "!"))
  ))

(add monster namer "monster-names")

(namer monster-names (random
  "Godzilla" "Rodan" "Mothra" "Megalon" "Gajira" "Aspidra"
  "Reptilicus" "Gamera"
  ))

;;; Random setup, for testing.

(add t* alt-percentile-min (  0  20  25  35  80  90 ))
(add t* alt-percentile-max ( 20  25  35  80  90 100 ))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)

(set country-radius-min 5)

(set sides-max 8)	; No more monsters ...

(add u* start-with (1 10 3 3 10 0 0 0))

(area 30 30)	; The monsters must be able to find each other.

(table favored-terrain
  (u* sea 0)
  )

(game-module (notes (
  "Typically, one would set up a scenario with one or more monsters on"
  "one side, and mobs, fire departments, and national guards on the"
  "other.  Note"
  "that the monster can easily defeat national guards one after another,"
  "and that the most successful strategy for the human side is to"
  "attack the monster with several units at once.  The monster can use"
  "fires as a barricade to keep the national guards from getting close"
  "enough to attack.  Destroying buildings is fun but not very useful."
  ""
  "Sandra Loosemore (sandra@cs.utah.edu) is the person to blame for this"
  "piece of silliness (well, Stan aided and abetted)."
  )))

