(game-module "insects"
  (title "Insects")
  ; (version "1.0")
  (picture-name "insects")
  (blurb "Bugs, Mr Rico! Zillions of 'em!")
  (variants (see-all false) (world-seen true) (sequential false))
  )

(unit-type ant (char "a")
  (help ""))
(unit-type spider (char "s")
  (help "eats bugs and weaves webs"))
(unit-type skeeter (char "k")
  (help "water strider, can go on land or water"))
(unit-type beetle (char "B")
  (help "slow-moving but powerful"))
(unit-type queen (image-name "crown") (char "Q")
  (help ""))
(unit-type fly (char "f")
  (help "fast-moving, for recon"))
(unit-type bee (char "b")
  (help "floats like a bee, stings like a bee, but only once before dying"))
(unit-type web (char "%")
  (help "a barrier to movement"))
(unit-type mound (char "*")
  (help "a secondary home"))
(unit-type nest (char "@")
  (help "the insect's home and breeding ground"))

(material-type mobility
  (help "the ability of a web to move before being anchored down"))
(material-type sting
  (help "what a bee attacks with"))

(terrain-type puddle (image-name "sea") (char "-"))
(terrain-type bare (image-name "adv-mountains") (char "^"))
(terrain-type grass (image-name "civ-grassland") (char "+"))

(define walkers (ant spider skeeter beetle))
(define movers (ant spider skeeter beetle queen fly bee))
(define flyers (queen fly bee))
(define places (mound nest))
(define land (bare grass))

(add puddle liquid true)

;;; Static relationships.

(table vanishes-on
  ((ant spider beetle) puddle true)
  (places puddle true)
  )

(table unit-capacity-x
  (spider web 2)
  )

(add queen capacity 2)
(add web capacity 1)
(add places capacity 8)

(table unit-size-as-occupant
  (u* u* 99)
  (movers (queen mound nest) 1)
  (walkers web 1)
  )

(table unit-storage-x
  (bee sting 1)
  (web mobility 1)
  )

;;; Actions.

;                     a s k B Q f b % * @
(add u* acp-per-turn (2 4 2 1 5 7 5 1 1 1))

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (u* t* 99)
  (walkers land 1)
  (skeeter puddle 1)
  (skeeter land 2)
  (flyers t* 1)
  (web t* 1)
  )

;; Web gets one move, for placement, then must stay there.

(table material-to-move (web mobility 1))

(table consumption-per-move (web mobility 1))

;;; Construction.

;               a s  k  B  Q f  b
(add movers cp (2 4 10 30 25 3 10))
(add web cp 5)

(table can-create
  (places movers 1)
  (mound queen 0)
  (nest queen 1)
  (spider web 1)
  )

(table can-build
  (places movers 1)
  (mound queen 0)
  (nest queen 1)
  (spider web 1)
  )

(table acp-to-create
  (places movers 1)
  (mound queen 0)
  (nest queen 1)
  (spider web 1)
  )

(table acp-to-build
  (places movers 1)
  (mound queen 0)
  (nest queen 1)
  (spider web 1)
  )

(table supply-on-creation
  (bee sting 1)
  (web mobility 1)
  )

;;; Combat.

;                a   s   k   B   Q   f   b  %   *   @
(add u* hp-max (20  50  20 100  20  10  20  2  20  50))

;; Injured insects move more slowly.

(add ant     speed-damage-effect ((1 50) ( 9  50) (10 100) ( 20 100)))
(add spider  speed-damage-effect ((1 50) (12  50) (13 100) ( 50 100)))
(add skeeter speed-damage-effect ((1 50) ( 9  50) (10 100) ( 20 100)))
(add beetle  speed-damage-effect ((1 50) (49  50) (50 100) (100 100)))
(add queen   speed-damage-effect ((1 50) ( 9  50) (10 100) ( 20 100)))
(add fly     speed-damage-effect ((1 50) ( 4  50) ( 5 100) ( 10 100)))
(add bee     speed-damage-effect ((1 50) ( 9  50) (10 100) ( 20 100)))

(table hit-chance
  ;               a   s   k   B   Q  f   b  %   *   @
  (ant u*     (  70  50  70  30  90  80 75 30 100 100 ))
  (spider u*  (  70  50  70  40  90  90 75 70 100 100 ))
  (skeeter u* (  70  50  70  30  90  80 75 50 100 100 ))
  (beetle u*  (  90  70  80  50  95  50 50 70 100 100 ))
  (queen u*   (  10  10  10  10  50  50 40 10 100 100 ))
  (fly u*     (  10  10  10  10  20  50 25 10  50  50 ))
  (bee u*     (  90  70  90  50  90  80 75 30 100 100 ))
  (web u*     (   0   0   0   0   0   0  0  0   0   0 ))
  (mound u*   (  20  20  20  20  20  20 20  0   0   0 ))
  (nest u*    (  50  50  50  50  50  50 50  0   0   0 ))
  )

(table damage
  (u* u* 1)
  (movers movers 2d6+4)
  (bee movers 3d6+6)
  )

(table protection
  (nest ant 50)
  )

(table capture-chance
  (web movers ( 50 30 50 20 70 70 50 ))
  (movers mound 20)
  )

(table independent-capture-chance
  (movers mound 80)
  )

;; Bees work by using their sting once in an attack, and then dying.

(table consumption-per-attack
  ;; (but should use even if not necessary to attack)
  (bee sting 1)
  )

(table hit-by
  (u* sting 1)
  )

(table material-to-attack
  (bee sting 1)
  )

(table hp-to-garrison
  (queen nest 20)
  )

;;; Other Actions.

(table auto-repair
  (mound mound 1.00)
  (nest nest 1.00)
  )

;;; Backdrop.

;; Insects generally have short lifespans.

(table attrition (movers t* 0.50))

(table hp-per-starve
  ;; Stinging something is fatal to the bee.
  (bee sting 20.00)
  )

;;; Scoring.

(add nest point-value 25)
(add web point-value 0)

(scorekeeper (do last-side-wins))

;;; Text.

(set action-notices '(
  ((destroy bee movers) (actor " stings " actee " to death!"))
  ((destroy u* movers) (actor " eats " actee "!"))
  ))

(set event-notices '(
  ((unit-starved web) (0 "dissolves"))
  ((unit-starved u*) (0 "expires"))
  ))

(set event-narratives '(
  ((unit-starved web) (0 "dissolved"))
  ((unit-starved u*) (0 "expired"))
  ))

;;; Random setup.

(set alt-smoothing 3)

(add t* alt-percentile-min (  0  10   0))
(add t* alt-percentile-max ( 15  60 100))
(add t* wet-percentile-min ( 50   0   0))
(add t* wet-percentile-max (100  80 100))

(set edge-terrain puddle)

(add nest start-with 1)

(set country-radius-min 2)

(add bare country-terrain-min 3)

(set country-separation-min 24)

(table independent-density (mound land 100))

(add mound independent-near-start 3)

(table favored-terrain
  (u* t* 0)
  (mound grass 50)
  (places bare 100)
  )

(add u* already-seen 100)

(add nest initial-seen-radius 5)

(set sides-max 4)

(set side-library '(
  ((noun "Buzzer"))
  ((noun "Flitter"))
  ((noun "Flyer"))
  ((noun "Hummer"))
  ))

;; Default doctrine.

(doctrine spider-doctrine
  (construction-run (web 3))
  )

(doctrine queen-doctrine
  (construction-run (nest 1))
  )

(side-defaults
  (doctrines (spider spider-doctrine) (queen queen-doctrine))
  )

(set meridians false)

;;; Documentation.

(game-module (instructions (
  "When you defeat your enemies, you get to eat them!"
  ""
  "Capture the other sides' nests."
  )))

(game-module (notes (
  "The nest is the center of your world; produce ants to "
  "expand with, spiders to build webs for defense, and skeeters "
  "to cross water."
  )))

(game-module (design-notes (
  "This was originally designed by Chris Christensen,"
  "inspired by a board game called `Chiten I'(?)."
  "It has been extensively modified since then."
  )))
