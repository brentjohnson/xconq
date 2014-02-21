(game-module "classic"
  (title "Classic Xconq")
  (version "1.0")
  (blurb "The standard game of Xconq 5. For old Xconq hands only.")
  (variants
    (world-seen false)
    (see-all false)
    (sequential false)
    (world-size (60 30 60))
    ("Own Initial Towns" own-initial-towns
       (true
        (add T start-with 5)
        (add T independent-near-start 0)
	    ))
    )
  )

(unit-type infantry (image-name "soldiers-s")
  (help "marches around and captures things"))
(unit-type armor (image-name "tank-s")
  (help "faster than infantry, limited to open terrain"))
(unit-type fighter (image-name "fighter-s")
  (help "interceptor to get those nasty bombers"))
(unit-type bomber (image-name "4e-s")
  (help "long range aircraft, carries infantry and bombs"))
(unit-type destroyer (image-name "dd-s")
  (help "fast, cheap, and sinks subs"))
(unit-type submarine (image-name "sub-s")
  (help "sneaks around and sinks ships"))
(unit-type troop-transport (name "troop transport") (image-name "ap-s")
  (help "carries infantry and armor across the pond"))
(unit-type carrier (image-name "cv-s") (char "C")
  (help "carries fighters and bombers around"))
(unit-type battleship (image-name "bb-s") (char "B")
  (help "the most powerful ship"))
(unit-type nuclear-bomb (name "nuclear bomb") (image-name "bomb-s") (char "N")
  (help "leveler of cities (and anything else)"))
(unit-type base (image-name "airbase-s") (char "/")
  (help "airbase plus port"))
(unit-type town (image-name "town20-s") (char "*")
  (help "smaller than a city"))
(unit-type city (image-name "city20-s") (char "@")
  (help "capital of a side"))

(material-type fuel)
(material-type ammo)

(terrain-type sea (char "."))
(terrain-type shallows (char ","))
(terrain-type swamp (char "="))
(terrain-type desert (char "~"))
(terrain-type plains (char "+"))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))
(terrain-type vacuum (char ":"))

(define i infantry)
(define a armor)
(define f fighter)
(define b bomber)
(define d destroyer)
(define s submarine)
(define t troop-transport)
(define cv carrier)
(define bb battleship)
(define nk nuclear-bomb)
(define B base)
(define T town)
(define @ city)

(define makers (T @))
(define ground (i a))
(define aircraft (f b))
(define ship (d s t cv bb))
(define cities (B T @))
(define movers (i a f b d s t cv bb nk))

(define water (sea shallows))
(define land (plains forest desert mountains))

(add water liquid true)

;;; Static relationships.

;;; Unit-unit.

(table unit-capacity-x
  ;; Part of a carrier's capacity is not available to bombers.
  (cv fighter 4)
  )

(add u* capacity (0 0 0 1 0 0 6 4 0 0 20 40 80))

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  ((i nk) b 1)
  (ground t 1)
  ((f b) cv (1 2))
  (u* B 2)
  (fighter B 1)
  (B B 100)
  (movers (T @) 1)
  ;; higher sizes for ships? used to be 6 for bbs, etc
  )

(table occupant-max (u* u* 100))

;;; Unit-terrain.

(table vanishes-on
  (ground water true)
  (cities water true)
  (ship land true)
  (u* ice true)
  (aircraft ice false)
  )

;; Always only one unit per cell. (If you want to stack units,
;; go play the standard game.)

(add t* capacity 1)

(table unit-size-in-terrain (u* t* 1))

;;; Unit-material.

(table unit-storage-x
  (u* fuel ( 6 10 18 36 100 100 200 400 200 1 200 500 900))
  (u* ammo ( 6  4  3  3  20  10  20  40  40 1 100 200 400))
  )

;;; Vision.

(add (T @) see-always true)

(add @ vision-range 3)

(table visibility
  (s t* 10)
  (nk t* 10)
  )

;;; Actions.

(add u* acp-per-turn  (1 2 9 6 3 3 2 4 4 1 0 1 1))

;;; Movement.

(add (B T @) speed 0)

(table mp-to-enter-terrain
  ;; Accident prevention.
  ((i a) (sea shallows) 99)
  (ship land 99)
  ;; Armor can only operate in open terrain.
  (a (swamp forest mountains) 99)
  ;; Big ships must move more slowly in shallow water.
  ((cv bb) shallows 2)
  )

;;; Everybody needs and uses fuel to move, except for nukes.

(table material-to-move
  (movers fuel 1)
  (nk fuel 0)
  )

(table consumption-per-move
  (movers fuel 1)
  (nk fuel 0)
  )

(table mp-to-enter-unit
  (u* u* 1)
  ;; Aircraft can't sortie again until next turn.
  (f u* 9)
  (b u* 6)
  (ship u* 0)
  (nk u* 0)
  ;; Armor can travel quickly on the many roads found around cities.
  (a (T @) 0)
  )

;; Don't be too picky about preventing movement of crippled
;; units.

(add u* free-mp 1)

;;; Construction.

(add nk tech-max 60)
(add nk tech-to-build 60)

(table acp-to-develop
  ;; only cities can develop nukes
  (@ nk 1)
  )

(table tech-per-develop
  ;; only cities can develop nukes
  (@ nk 1.00)
  )

(table can-toolup-for
  ((T @) u* true)
  )

(table acp-to-toolup
  ((T @) u* 1)
  )

(table tp-to-build
  (T movers (1 2 2 3 2 3 2 6 8 4))
  (@ movers (1 2 2 3 2 3 2 6 8 4))
  )

(table tp-max
  (T movers (1 2 2 3 2 3 2 6 8 4))
  (@ movers (1 2 2 3 2 3 2 6 8 4))
  )

(add u* cp (4 7 8 16 10 16 12 30 40 20 3 1 1))

(table can-create
  (ground B 1)
  ((T @) movers 1)
  )

(table acp-to-create
  (ground B 1)
  ((T @) movers 1)
  )

(table cp-on-creation
  (ground B 1)
  ((T @) movers 1)
  )

(table can-build
  (ground B 1)
  ((T @) movers 1)
  )

(table acp-to-build
  (ground B 1)
  ((T @) movers 1)
  )

(table cp-per-build
  (ground B 1)
  ((T @) movers 1)
  )

;;; Combat.

(add u* hp-max (1 1 1 2 3 2 3 4 8 1 10 20 40))

;; Units are generally crippled, meaning they move at half speed,
;; at about 1/2 of hp-max, sometimes rounding up, sometimes down.

(add b  speed-damage-effect ((1 50) (2 100)))
(add d  speed-damage-effect ((1 50) (2 100) (3 100)))
(add s  speed-damage-effect ((1 50) (2 100)))
(add t  speed-damage-effect ((1 50) (2 50) (3 100)))
(add cv speed-damage-effect ((1 50) (2 50) (3 100) (4 100)))
(add bb speed-damage-effect ((1 50) (4 50) (5 100) (8 100)))

;(add movers speed-min    (0 0 0 3 3 3 2 1 2 1))

(table acp-to-attack
  (u* u* 1)
  ;; Places cannot initiate attacks, although they will certainly
  ;; hit back when attacked.
  ((B T @) u* 0)
  )

;;; The main hit chance table.

(table hit-chance
  (i  u* ( 50  40  20  15  20  20  30  20   9 100  80  60  40))
  (a  u* ( 60  50  30  30  30  20  30  20  20 100  90  70  50))
  (f  u* ( 15  25  60  70  20  30  20  50  40 100 100 100 100))
  (b  u* ( 20  20  10   9  30  50  50  70  60 100  90  95  99))
  (d  u* (  5   5  10   5  60  70  60  40  20 100  99  90  80))
  (s  u* (  0   0  10   5  40  10  60  40  50 100   0   0   0))
  (t  u* ( 20   5  10   5  40  40  40  30   9 100   0   0   0))
  (cv u* ( 30  20  40  10  30  30  40  20  20 100   0   0   0))
  (bb u* ( 50  50  50  20  70  50  90  50  90 100 100 100 100))
  (nk u* ( 99  99  99  99  99  99  99  99  99 100  99  99  99))
  (B  u* ( 10  10  20  20  20  20  30  20  20 100   0   0   0))
  (T  u* ( 30  20  50  40  40   0  30  20  20 100   0   0   0))
  (@  u* ( 50  40  70  60  50   0  30  20  50 100   0   0   0))
  )

;;; The main damage table.

(table damage
  (u* u* 1)
  (a cities 2)
  (b ship 2)
  (b s 1)
  (b cities (2 2 3))
  (d s 2)
  (s ship 3)
  (s bb 4)
  (bb u* 2)
  (bb (T @) (3 4))
  (nk u* 60)   ; wham!
  )

(table capture-chance
  (i cities (70 50 30))
  (a cities (90 70 50))
  )

(table independent-capture-chance
  (i cities (100 80 50))
  (a cities (100 95 70))
  )

(table bridge
  ;; Infantry can capture cities even on water.
  (i cities true)
  )

(table protection ; actually ablation?
  ; cities offer some protection to occupants
  (cities movers 50)
  ; inf and armor protect the cities housing them.
			  ; can't make this too large or city can be
			  ; invulnerable.
  (i cities 5)
  (a cities 10)
  ; bases benefit more from protection.
  ((i a) B (15 25))
  )

;; We need ammo to hit with.

(table consumption-per-attack (u* ammo 1))

(table hit-by (u* ammo 1))

;; Nuclear bombs in 5.x attack in a way that is half-attack
;; and half-detonation, partly because of limitations in the
;; damage calculations.  Neither attack nor detonation in 7.x
;; can be completely identical in effect; I chose attack as
;; being more compatible, although it cannot cause any terrain
;; changes.

;;; Texts.

(set action-notices '(
  ((destroy u* ground) (actor " defeats " actee "!"))
  ((destroy u* aircraft) (actor " shoots down " actee "!"))
  ((destroy u* ship) (actor " sinks " actee "!"))
  ((destroy u* cities) (actor " flattens " actee "!"))
  ))

;;; Other Actions.

;; In 5.x, repair is always automatic.

(table auto-repair
  ((T @) u* 1.00)
  (B u* 3.00)
  ((cv bb) (cv bb) 10.00)
  (i cities (1.00 10.00 10.00))
  )

(table auto-repair-range (u* u* -1))

(add movers acp-to-disband 1)

;; This does everybody in one shot.

(add movers hp-per-disband 10)

;; Takes a while to dismantle a base.

(add B hp-per-disband 2)

;; Only infantry cannot change hands.

(add u* acp-to-change-side 1)
(add i acp-to-change-side 0)

;; And it cannot be independent.

(add i possible-sides (not "independent"))

;;; Backdrop economy.

(table base-production
  (ground fuel 1)
  (cities fuel (10 20 50))
  (cities ammo (5 10 20))
  )

(table productivity
  (i (desert mountains) 0)
  (a (desert forest mountains) 0)
  (B land (100 50 20 20))
  (T land (100 50 20 20))
  (@ land (100 50 20 20))
  )

; fighter fuel consumption is problem for carriers, should probably
; make carrier prod high or else bump up capacity
; (on the other hand, high consumption *is* more realistic...)

(table base-consumption
  ((i f b) fuel (1 3 2))
  (ship fuel 1)
  )

(table consumption-as-occupant
  ;; Aircraft that are in something else are not flying.
  ((f b) fuel 0)
  )

(table hp-per-starve
  ((i f b) fuel 1.00)
  (ship fuel 10.00)
  )

(table out-length
  ;; These types should never give up any of their supplies.
  ((i a f b nk) m* -1)
  )

;;; Scoring.

(add (T @) point-value (1 5))

(scorekeeper (do last-side-wins))

;;; Setup.

(set synthesis-methods
  '(make-fractal-percentile-terrain make-countries make-independent-units
    make-initial-materials name-units-randomly name-geographical-features))

(set alt-blob-density 5000)
(set alt-blob-size 40)
(set wet-blob-density 1000)
(set wet-blob-size 100)

(add t* alt-percentile-min (  0  68  69  70  70  70  93  99  0))
(add t* alt-percentile-max ( 68  69  71  93  93  93  99 100  0))
(add t* wet-percentile-min (  0   0  50   0  20  80   0   0  0))
(add t* wet-percentile-max (100 100 100  20  80 100 100 100  0))

(set edge-terrain ice)

(add @ start-with 1)
(add T independent-near-start 5)
(set country-radius-min 2)
(set country-separation-min 16)
(set country-separation-max 48)
;; Try to get countries on the coast.
(add (sea plains) country-terrain-min (3 6))

(table favored-terrain
  (u* t* 0)
  (@ plains 100)
  (T land 20)
  (T plains 40)
  )

(table independent-density
  (town (plains desert mountains forest) (100 50 50 50))
  )

(table unit-initial-supply
  (u* fuel ( 6 10 18 36 100 100 200 400 200 1 200 500 900))
  (u* ammo ( 6  4  3  3  20  10  20  40  40 1 100 200 400))
  )

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


;;; Documentation.

(game-module (instructions (
  "If you've played version 5, then just play this in the same way."
  "If you've never played version 5, choose the `standard game' instead."
  )))

(game-module (notes (
  "(This attempts to be an accurate rendition of the 5.x standard game.)"
  "(The old notes follow:)"
  ""
  "This game is the most commonly played Xconq game.  It"
  "represents units of about 1945, from infantry to atomic bombs.  This is"
  "familiar, which makes it easier to play, but also more controversial,"
  "since historians have many conflicting theories about which kinds of"
  "units were most effective."
  ""
  "Opinions differ about optimal strategy for this game.  In general,"
  "blitzkrieg works, and can win the game in a hurry.  The problem is to"
  "muster enough force before striking.  One full troop transport is not"
  "enough; the invasion will melt away like ice cream on a hot sidewalk,"
  "unless"
  "reinforcements (either air or land) show up quickly.  Air cover is very"
  "important.  While building up an invasion force, airborne assaults using"
  "bombers and infantry can provide useful diversions, although it can be"
  "wasteful of bombers.  Human vs human games on a 60x30 world generally"
  "last about 100 turns, usually not enough time or units to build atomic"
  "bombs or battleships, and not a big enough world to really need carriers"
  "(although bases for staging are quite useful)."
  )))

(add i notes (
  "The infantry division is the slowest of units, but it can go almost"
  "anywhere.  It is also quick to produce.  Infantry is the staple of"
  "campaigns - a little boring perhaps, but essential to success."
  ))
(add a notes (
  "The armor division is highly mobile and hits hard.  Unfortunately,"
  "it is limited to operating in open terrain - plains and desert.  It also"
  "takes longer to produce.  Armor can last twice as long in the  "
  "desert as infantry.  Both armor and infantry can"
  "assault and capture cities; they are the only units that can do so."
  ))
(add f notes (
  "A fighter is a squadron or wing of high-speed armed aircraft."
  "Their fuel supply can be gotten only at units, towns, and bases, so they"
  "must continually be taking off and landing.  Fighters are not too effective"
  "against ground units or ships, but they eat bombers for lunch.  Fighters"
  "are very good for reconnaisance - important when you can't always"
  "see the enemy moving!"
  ))
(add b notes (
  "Bombers are very powerful, since they can seriously damage"
  "or even flatten cities.  Loss rate in such activities is high, so they're"
  "not a shortcut to victory!"
  "Bombers can carry one infantry, which is very useful for raiding."
  ))
(add d notes (
  "Destroyers are fast small ships for both exploration and"
  "anti-submarine activities."
  ))
(add s notes (
  "The favorite food of submarines is of course merchant shipping"
  "and troopships, and they can sink troop transports with one blow."
  "Subs are also invisible, but vulnerable to destroyers and aircraft."
  ))
(add t notes (
  "This is how ground units get across the sea.  They can"
  "defend themselves against ships and aircraft, but are basically vulnerable."
  "They're not very fast either."
  ))
(add cv notes (
  "Compensates for the fighter's limited range by providing"
  "a portable airport.  Carriers themselves are sitting ducks, particularly"
  "with respect to aircraft.  Fighter patrols are mandatory."
  ))
(add bb notes (
  "The aptly named `Dread Naught' has little to fear from other"
  "units of this period.  Subs may sink them with enough effort, and a group"
  "of bombers and fighters are also deadly,"
  "but with eight hit points to start,"
  "a battleship can usually survive long enough to escape."
  "Battleships are very"
  "effective against cities and armies, at least the ones on the coast."
  ))
(add nk notes (
  "Atomic bombs.  The Final Solution; but they are not easy to use.  A bomb"
  "takes a long time to produce, moves very slowly by itself, and is easily"
  "destroyed by other units. The plus side is instant destruction for any unit"
  "of any size!  Bombs are imagined to be transported by a team of scientists,"
  "and so can go on any sort of terrain without running out of supplies."
  ))
(add B notes (
  "To simplify matters, this can serve as a camp, airbase, and port."
  "Bases cannot build units, although they can repair some damage."
  ))
(add T notes (
  "Towns are the staple of territory.  They can build, repair, produce"
  "fuel and ammo, and serve as a safe haven for quite a few units."
  ))
(add @ notes (
  "Cities are very large, powerful, and well defended.  They are"
  "basically capital cities, or something in a comparable range.  (New York"
  "and San Francisco are cities, Salt Lake City and San Antonio are towns."
  "Yeah, San Antonio has a lot of people, but it's still insignificant,"
  "nyah nyah.)  A city is worth five towns, territory-wise."
  ))
