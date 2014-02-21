(game-module "lhsunit"
  (blurb "Standard unit types and tables modified for the modern world")
  )

;;; New format version of the standard game.  This file is just types and
;;; tables, it is not a complete game (see "lhs" for that).

(unit-type infantry (image-name "soldiers")
  (help "marches around and captures things"))
(unit-type armor (image-name "tank")
  (help "faster than infantry, limited to open terrain"))
(unit-type fighter (image-name "fighter")
  (help "interceptor to get those nasty bombers"))
(unit-type bomber (image-name "4e")
  (help "long range aircraft, carries infantry and bombs"))
(unit-type destroyer (image-name "dd")
  (help "fast, cheap, and sinks subs"))
(unit-type submarine (image-name "sub")
  (help "sneaks around and sinks ships"))
(unit-type troop-transport (name "troop transport") (image-name "ap")
  (help "carries infantry and armor across the pond"))
(unit-type carrier (image-name "cv") (char "C")
  (help "carries fighters and bombers around"))
(unit-type battleship (image-name "bb") (char "B")
  (help "the most powerful ship"))
(unit-type nuclear-bomb (name "nuclear bomb") (image-name "bomb") (char "N")
  (help "leveler of cities (and anything else)"))
(unit-type base (image-name "airbase") (char "/")
  (help "airbase plus port"))
(unit-type town (image-name "town20") (char "*")
  (help "smaller than a city"))
(unit-type city (image-name "city20") (char "@")
  (help "capital of a side"))


;;;
;;; new unit types for the modern world
;;;
(unit-type radar (image-name "radar") (char "r")
  (help "looks for units"))
(unit-type artillery (image-name "pz-how") (char "A")
  (help "blows things up at a distance"))
(unit-type aaa (image-name "pz-flak") (char "F")
  (help "blows up aircraft"))
(unit-type engineer (image-name "engr") (char "e")
  (help "builds things"))
(unit-type mine (image-name "minefield") (char "m")
  (help "blows things up as they pass"))
(unit-type mineship (image-name "pt") (char "M")
  (help "lays and clears mines at sea"))

(define i infantry)
(define a armor)
(define f fighter)
(define b bomber)
(define d destroyer)
(define r radar)
(define s submarine)
(define t troop-transport)
(define cv carrier)
(define bb battleship)
(define nuke nuclear-bomb)
(define B base)
(define T town)
(define @ city)
(define A artillery)
(define e engineer)
(define m mine)
(define ms mineship)

(material-type fuel
  (help "basic supply that all units need"))
(material-type ammo
  (help "generic supply used in combat"))

(include "stdterr")

(define ground (i a r A aaa e))
(define aircraft (f b))
(define ship (d s t cv bb ms))
(define cities (T @))
(define places (B T @ m))
(define movers (i a f b d r s t cv bb nuke A aaa e ms))

(define water (sea shallows))
(define land (swamp plains forest desert mountains))

;;; Static relationships.

(table vanishes-on
  (ground water true)
  (armor swamp true)
  (places water (true true true false))
  (ship land true)
  (ship road true)
  ;; Only aircraft can deal with ice.
  (u* ice true)
  (aircraft ice false)
  )

;; Unit-unit capacities.

(add b capacity 2)
(add t capacity 8)
(add cv capacity 10)
(add places capacity (20 40 80 20))

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  ;; Bombers can carry two infantry or one tank, nuke, radar.
  ((i e a r nuke A aaa) b (1 2 2 2 2 2 2))
  ;; Transports can carry armor or infantry.
  (ground t 1)
  (aircraft cv 1)
  ;; Bases can hold 10 of most unit types, but up to 20 fighters.
  (u* B 2)
  (fighter B 1)
  ;; City types have lots of capacity for everything.
  (movers cities 1)
  (e m 2)
  )

(table occupant-max (u* u* 99))

;;; Unit-terrain capacities.

;; Limit units to 16 in one cell, for the sake of playability and
;; and drawability.  Places cover the entire cell, however.
  
(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  ;; Allow aircraft to pass over towns instead of landing.
  (aircraft t* 0)
  (places t* (16 16 16 10))
  )

;;; Unit-material capacities.

;;; column order is: infantry armor fighter bomber destroyer sub
;;;                  transport carrier battleship nuke base town city radar artillery aaa engineers mine mineship
(table unit-storage-x
  (u* fuel ( 6 10 20 30 100 100 150 500 400 100 200 500 900 10 10 10 10 0 50))
  (u* ammo ( 6  4  4  6  20  10  40  80  60   0 100 200 400  0 10 10  6 1  6))
  )

;;; Vision.

;; Towns and cities always have foreign correspondents, telephones,
;; private citizens coming and going, so their state is always
;; going to be available to any side that knows they exist.

(add cities see-always true)

;; Cities have more powerful radar and sensing systems, and
;; so they can see out further.

(add @ vision-range 4)
(add aaa acp-to-fire 1)
(add aaa range 3)
(add aaa vision-range 3)
(add artillery acp-to-fire 1)
(add artillery range 3)
(add artillery vision-range 3)
(add b vision-range 2)
(add bb acp-to-fire 1)
(add bb range 3)
(add bb vision-range 3)
(add cv vision-range 3)
(add d acp-to-fire 1)
(add d range 2)
(add d vision-range 2)
(add f acp-to-fire 1)
(add f range 2)
(add f vision-range 2)
(add r vision-range 4)
(add s acp-to-fire 1)
(add s range 2)
(add s vision-range 3)

(table see-chance-adjacent
  ;; Submarines are always hard to see.
  (u* s 40)
  ;; submarines are not good at seeing planes
  (s aircraft 20)
  (s ground 40)
  ;;
  (d ground 40)
  ;; A bomb in a truck is rather small and inconspicuous.
  (u* nuke 10)
  ;; mines are always hard to see.
  (u* m 10)
  )

;;; Actions.

;;; column order is: infantry armor fighter bomber destroyer sub
;;;                  transport carrier battleship nuke base town city radar artillery aaa engineer mine mineship
(add u* acp-per-turn  (1 2 9 6 3 3 3 4 4 1 0 1 1 1 1 1 1 1 1))

;;; Movement.

(add places speed 0)

;; Don't be too picky about mp usage.

(add u* free-mp 1)

;; Aircraft should always be able to land, and this in
;; conjunction with the entry cost will result in airplanes
;; being able to land at any time during their time, but
;; always having to wait until the next turn to take off
;; again.

(add aircraft free-mp (9 6))

(table mp-to-enter-terrain
  ((cv bb s) shallows 2)
  (a (swamp forest mountains) 99)
  ;; No special trouble to get on a road
  ;; (such as when using to cross a river).
  (a road 0)
  (A (swamp forest mountains) 99)
  (A road 0)
  (aaa (swamp forest mountains) 99)
  (aaa road 0)
  (ground water 99)
  (ship land 99)
  ;; Don't let ships use roads.  This might seem unnecessary,
  ;; but consider the case of a ship in a town wanting to leave
  ;; in a direction that has a road.  If the road appears to be
  ;; cheaper and the ship has enough mp, it will take the road
  ;; and then vanish.
  (ship road 99)
  (u* ice 99)
  (aircraft ice 1)
  )

(table mp-to-leave-terrain
  ;; This is for accident prevention, in the case of a ground
  ;; unit on a road bridge that doesn't quite reach land.
  (ground water 99)
  )

(table mp-to-traverse (a road 1))

(table material-to-move
  (movers fuel 1)
  (nuke fuel 0)
  )

(table mp-to-enter-unit
  (u* u* 1)
  ; aircraft can't sortie again until next turn
  ;(f u* 9)
  ;(b u* 6)
  ;; Some cost to land, but can still take off again in the same turn.
  (f u* 2)
  (b u* 2)
  (ship u* 0)
  (a cities 0)   ; travel quickly on surrounding roads.
  )

(table consumption-per-move
  (movers fuel 1)
  ;; Nukes are "important", a country would never risk them running out.
  (nuke fuel 0)
  )

;;; Construction.

;; Nuclear weapons must be developed before they can be built.

(add nuke tech-max 60)
(add nuke tech-to-build 60)

;; Only cities can develop nukes.

(table acp-to-develop (@ nuke 1))

(table tech-per-develop (@ nuke 1.00))

;; Limit the amount of gain possible due to a concentrated effort.
;; Note that this will only have an effect in games with many cities.

(add nuke tech-per-turn-max 3)

;; Basically, units take about as many turns to complete as their cp.

;;; column order is: infantry armor fighter bomber destroyer sub
;;;                  transport carrier battleship nuke base town city radar artillery aaa engineer mine mineship
(add u* cp (1 3 4 5 6 10 5 15 17 20 4 1 1 6 2 2 2 1 5))

(table can-create
  (cities movers 1)
  (e B 1)
  (e m 1)
  (ms m 1)
  )

(table acp-to-create
  (cities movers 1)
  (e B 1)
  (e m 1)
  (ms m 1)
  )

(table cp-on-creation
  (e B 1)
  )

(table can-build
  (e B 1)
  (e m 1)
  (cities movers 1)
  (ms m 1)
  )

(table acp-to-build
  (e B 1)
  (e m 1)
  (cities movers 1)
  (ms m 1)
  )

(table cp-per-build
  (a B 2)
  (e B 1)
  (e m 1)
  (ms m 1)
  )

(table occupant-can-construct (u* u* false))

;;; Repair.

;; Explicit repair actions accelerate the recovery of lost hp.

(table can-repair
  (cities u* true)
;  (B u* true)
  (cv cv true)
  (bb bb true)
  (i places (true true true false))
  (e places (true true true false))
  (e u* true)
  (m u* false)
  )

(table acp-to-repair
  (cities u* 1)
;  (B u* 1)
  (cv cv 1)
  (bb bb 1)
  (i places (1 1 1 0))
  (e places (2 2 2 0))
  (e u* 1)
  (m u* 0)
  )

(table hp-per-repair
  ;; Towns and cities can repair anything.
  (cities u* 1.00)
;  (B u* 3.00)  ; what is this all about?
  ;; Capital ships are equipped to do major repairs to themselves.
  (cv cv 1.00)
  (bb bb 1.00)
  ;; cariers can repair aircraft
  (cv aircraft 1.00)
  ;; The army's engineers can do lots of repair work if put to the task.
  (e places (2.00 5.00 5.00 0.00))
  (e u* 1.00)
  )

(table auto-repair
  ;; Capital ships are equipped to do major repairs to themselves.
  (cv cv 0.50)
  (bb bb 0.50)
  (cv aircraft 0.50)
  (cities movers 0.50)
  (e u* 1.00)
  )

;; Some types have sufficient people and redundancy to do some repair work
;; without affecting their readiness to act.

(add (cv bb B) hp-recovery 0.50)
(add cities hp-recovery 1.00)

;;; Production.

(table base-production
  (ground fuel 1)
  ;; This is not too realistic, but otherwise keeping tanks fueled
  ;; is a major hassle.  This is supposed to be a game, not an
  ;; exercise in logistics (play "empire.g" if you want that).
  (a fuel 2)
  (places fuel (10 20 50 0))
  (places ammo (5 10 20 0))
  )

(table productivity
  ;; Plains are assumed to be settled and have fuel stocks.
  (i (swamp desert mountains) 0)
  ;; It Tis* worthwhile to make players think about logistics
  ;; when operating in the desert.
  (a desert 0)
  (B land (0 100 50 20 20))
  (T land (0 100 50 20 20))
  (@ land (0 100 50 20 20))
  )

(table base-consumption
  ;; Note that armor does Tnot* have a base consumption;
  ;; tanks that don't move don't need fuel.  (Infantry
  ;; still needs fuel, because an infantry unit in real life
  ;; has a large number of personnel, and fuel represents
  ;; their general energy and food consumption.)
  ((i f b) fuel (1 3 2))
  (ship fuel 1)
  )

(table hp-per-starve
  ((i f b) fuel 1.00)
  ;; Immobilized tanks eventually rust...
  (armor fuel 0.05)
  (ship fuel 0.10)
  (places fuel (0.05 0.05 0.05 0.0))
  (m ammo 0.25)
  )

(table consumption-as-occupant
  ;; Aircraft, radar on the ground or in a carrier just sit there.
  ((f b r) fuel 0)
  )

;;; Combat.

(add u* hp-max (1 2 1 2 3 2 3 4 8 1 10 20 40 1 1 1 1 1 1))

;; Units are generally crippled, moving at half speed,
;; at about 1/2 of hp-max, sometimes rounding up, sometimes down.

(add b  speed-damage-effect ((1 50) (2 100)))
(add d  speed-damage-effect ((1 50) (2 100) (3 100)))
(add s  speed-damage-effect ((1 50) (2 100)))
(add t  speed-damage-effect ((1 50) (2  50) (3 100)))
(add cv speed-damage-effect ((1 50) (2  50) (3 100) (4 100)))
(add bb speed-damage-effect ((1 50) (4  50) (5 100) (8 100)))

;;; The main hit table.
;;; column order is: infantry armor fighter bomber destroyer sub
;;;                  transport carrier battleship nuke base town city radar artillery aaa engineer mine mineship

(table hit-chance
   (i u* ( 50  40  20  15   0   0  10  10   5  40  80  60  40  60  50  50  60  20  20))
   (a u* ( 75  50  20  20  10   0  20  20  20  50  90  70  50  60  65  65  85   0  30))
   (f u* ( 20  55  60  70  30   0  40  35  35  70  70  60  60  25  50  50  50   0  55))
   (b u* ( 40  45  10   5  60  30  50  50  60  50  80  80  80  60  70  70  60   0  60))
   (d u* (  5   5  20  15  50  80  60  20  10   0  80  70  70   5   5   5  15   0  60))
   (s u* (  0   0   0   0  85  80  90  50  60   0   0   0   0   0   0   0   0   0  90))
   (t u* ( 20   5   5  10  10   5  20   5   5   0   0   0   0   0   5   5  30   0  30))
  (cv u* (  0   0  40  30  20  10  40  20  20   0   0   0   0   0   0   0   0   0  40))
  (bb u* ( 85  60  30  20  90  10  90  90  80   0 100  90  90  50  75  75  80   0  90))
(nuke u* (100 100 100 100 100 100 100 100 100 100 100 100 100 100 100 100 100   0 100))
  (B  u* ( 10  10  20  20  20   0  30  20  20   0   0   0   0   0  15  15  20   0  30))
  (T  u* ( 30  20  50  40  40   0  30  20  50   0   0   0   0   0  25  25  40   0  30))
  (@  u* ( 50  40  70  60  50   0  30  20  50   0   0   0   0   0  45  45  60   0  30))
  (r  u* (  0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0))
   (A u* ( 85  65   0   0  15   0  10  30  40  50  95  80  70  60  80  80  95   0  40))
 (aaa u* (  0   0  60  70   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0))
   (e u* (  0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0  90   0))
   (m u* ( 70  80   0   0  60  50  70  70  70   0   0   0   0   0   0   0   0   0   0))
  (ms u* (  0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0  90   0))
  )
;;; column order is: infantry armor fighter bomber destroyer sub
;;;                  transport carrier battleship nuke base town city radar artillery aaa engineer mine mineship

(table damage
  (u* u* 1)
  (a places (2 2 2 0))
  (b ship 2)
  (b s 1)
  (b places (2 2 3 0))
  (d s 2)
  (s ship 3)
  (s bb 4)
  (bb u* 2)
  (bb cities (3 4))
  (A u* 2)
  (A cities (2 2))
  (m u* 3)
  (aaa f 2)
  (aaa b 2)
  )

(table capture-chance
  (i places (70 50 30 0))
  (a places (90 70 50 0))
  )

(table independent-capture-chance
  (i places (100 80 50 0))
  (a places (100 95 70 0))
  )

;; Intelligent and/or valuable units will work to preserve themselves.

(table retreat-chance
  (a i 10)
  (a a 20)
  (f f 20)
  (f b 50)
  (ship cv 50)
  )

;; infantry can capture cities even on water.

(table bridge (i places true))

(table protection ; actually ablation?
  ;; places offer some protection to occupants
  (places movers (50 50 50 0))
  ;; but bases make aircraft sitting ducks, so no protection there.
  (base aircraft 100)
  ;; inf and armor protect the places housing them.
  ;; can't make this too large or city can be
  ;; invulnerable.
  (i places (80 80 80 0))
  (a places (60 60 60 0))
  ; bases benefit more from protection.
  (ground B (50 40 0 0 0 0))
  )

;; Combat requires ammo, and uses it up.

(table consumption-per-attack (u* ammo 1))

(table consumption-per-fire 
	((aaa artillery bb d f s) ammo 1)
)

(table hit-by (u* ammo 1))

;;; Detonation.

;; Nukes work by detonation rather than by conventional attack actions.

(add nuke acp-to-detonate 1)

(add nuke hp-per-detonation 1)

(table detonation-damage-at (nuke u* 60))

(table detonation-damage-adjacent (nuke u* 1))

(table detonation-terrain-damage-chance
  (nuke (plains forest) 100)
  )

(table terrain-damaged-type
  (plains desert 1)
  (forest desert 1)
  )

;;; Disbanding.

;; This does movers in one shot except battleships, which historically
;; are usually difficult to scuttle.

(add movers acp-to-disband 1)
(add bb acp-to-disband 2)

(add movers hp-per-disband 4)

;; Takes a while to dismantle a base.

(add B hp-per-disband 2)

;;; Changing sides.

(add u* acp-to-change-side 1)
(add i acp-to-change-side 0)

(add i possible-sides (not "independent"))

;;; Automatic things.

;; Economy.

(table out-length
  ;; Net consumers of supply should never give any up automatically.
  ((i a b f nuke m) m* -1)
  ;; Cities and towns can share things around.
  (cities m* 1)
  (e m* 1)
  )

(table in-length
  ;; Supply to ground units can go a couple hexes away.
  (ground m* 3)
  ;; Cities and bases can get their supplies from some distance away.
  (B m* 6)
  (cities m* 12)
  (m m* 1)
  )

;;; Scoring.

;; Most units aren't worth much, but ground units can capture
;; cities, carriers and battleships are powerful, and nukes are
;; devastating, especially when stockpiled.

(add u* point-value 0)
(add (a A aaa f b s cv bb) point-value (1 1 1 2 2 2 3 3))
(add nuke point-value 5)
(add cities point-value (5 25))

;;; Texts.

(set action-notices '(
  ((disband infantry self done) (actor " goes home"))
  ((disband u* bomb done) (actor " dismantles " actee))
  ((disband u* u* done) (actee " disbands"))
  ))

(set event-notices '(
  ((unit-starved fighter) (0 " runs out of fuel and crashes!"))
  ((unit-starved bomber) (0 " runs out of fuel and crashes!"))
  ))

(set event-narratives '(
  ((unit-starved fighter) (0 " ran out of fuel and crashed"))
  ((unit-starved bomber) (0 " ran out of fuel and crashed"))
  ))

;;; Initial setup.

(add cities start-with (8 3))
(set country-radius-min 6)
(set country-separation-min 25)
(set country-separation-max 48)
;; Try to get countries to be on the coast.
(add (sea plains) country-terrain-min (1 4))

(table favored-terrain
  (u* t* 0)
  (@ plains 100)
  (T land 20)
  (T plains 40)
  )

(table independent-density (town plains 100))

(add land country-people-chance 90)
(add plains country-people-chance 100)

(add land independent-people-chance 50)

(table road-chance
  (city (town city) (80 100))
  (town (town city) ( 2   5))
  )

(add (town city) road-to-edge-chance 100)

(set edge-road-density 100)

;; Nearly all towns should be connected by road to
;; somewhere else.

(add town spur-chance 90)
(add town spur-range 2)

;; A game's starting units will be full by default.

(table unit-initial-supply
  (u* m* 9999)
  (m m* 0)
  )

;; Default doctrine.

(doctrine default-doctrine
  (construction-run (u* 1))
  (rearm-percent 40)
  )

(doctrine place-doctrine
  (construction-run (u* 99) ((carrier battleship) 3) (nuke 1)))

(side-defaults
  (default-doctrine default-doctrine)
  (doctrines (places place-doctrine))
  )

(game-module (notes (
  "This game is modified from the standard one in Xconq.  It"
  "is an attempt to bring the 1945 game up to post-cold-war"
  "military technology. The main changes are that everything"
  "is more destructive, ranges are greater, and submarines and"
  "aircraft are much more powerful."
  )))

(add i notes '(
  "The infantry army is the slowest of units, but it can go almost"
  "anywhere.  It is also quick to produce.  Infantry is the staple of"
  "campaigns - no special features, but quick and cheep."
  ))
(add a notes '(
  "The armor army is highly mobile and hits hard.  Unfortunately,"
  "it is limited to operating in open terrain - plains and desert.  It also"
  "takes longer to produce.  Armor can last twice as long in the  "
  "desert as infantry.  Both armor and infantry can"
  "assault and capture cities; they are the only units that can do so."
  ))
(add f notes '(
  "A fighter is a squadron or wing of high-speed armed aircraft."
  "Their fuel supply can be gotten only at units, towns, and bases, so they"
  "must continually be taking off and landing.  Fighters are effective"
  "against ground units and ships, and they eat bombers for lunch.  Fighters"
  "are very good for reconnaisance - important when you can't always"
  "see the enemy moving!"
  ))
(add b notes '(
  "Bombers are very powerful, since they can seriously damage"
  "or even flatten cities.  Loss rate in such activities is high, so they're"
  "not a shortcut to victory!"
  "Bombers can carry two infantry or an armor, which is very useful for raids."
  ))
(add d notes '(
  "Destroyers are fast small ships for both exploration and"
  "anti-submarine activities."
  ))
(add s notes '(
  "The favorite food of submarines is of course merchant shipping"
  "and troopships, and they can sink troop transports with one blow."
  "Subs are also invisible, but vulnerable to destroyers and aircraft."
  ))
(add t notes '(
  "This is how ground units get across the sea.  They can"
  "defend themselves against ships and aircraft, but are basically vulnerable."
  "They're not very fast either."
  ))
(add cv notes '(
  "Compensates for the limited range of fighters and bombers by providing"
  "a portable airport.  Carriers themselves are sitting ducks, particularly"
  "with respect to aircraft.  Fighter patrols are mandatory."
  ))
(add bb notes '(
  "This may be the most controversial change in the game. As a reflection"
  "of modern targeting systems, the effective range of battleships is now"
  "longer, so they can destroy a city from over the horizon. They are"
  "very powerful, but can still be destroyed by aircraft and subs."
  ))
(add nuke notes '(
  "Atomic bombs.  The Final Solution; but they are not easy to use.  A bomb"
  "takes a long time to produce, moves very slowly by itself, and is easily"
  "destroyed by other units. The plus side is instant destruction for any unit"
  "of any size!  Bombs are imagined to be transported by a team of scientists,"
  "and can go on any sort of terrain without running out of supplies."
  ))
(add B notes '(
  "To simplify matters, this can serve as a camp, airbase, and port."
  "Bases cannot build units, although they can repair some damage."
  ))
(add T notes '(
  "Towns are the staple of territory.  They can build, repair, produce"
  "fuel and ammo, and serve as a safe haven for quite a few units."
  ))
(add @ notes '(
  "Cities are very large, powerful, and well defended.  They are"
  "basically capital cities, or something in a comparable range.  (New York"
  "and San Francisco are cities, Salt Lake City and San Antonio are towns."
  "Yeah, San Antonio has a lot of people, but it's still insignificant,"
  "nyah nyah.)  A city is worth five towns, point-wise."
  ))
(add r notes '(
  "radar is a passive unit that sees a great distance. While fragile and"
  "without any attacking abilities, they can still play a role, especially"
  "if loaded onto a bomber to produce an AWACS."
  ))
(add A notes '(
  "Artillery are units of mobile guns, more powerful but much more fragile"
  "than tanks. By themselves, they can not capture anything, but they can"
  "destroy almost any ground or sea unit they can see."
  ))
(add aaa notes '(
  "Anti-aircraft artillery are fragile, but can chew up fighters and bombers"
  ))
(add e notes '(
  "Engineer units are dull but essential. While they are useless in a fight,"
  "they can build bases, repair damaged units, and lay or remove mine fields."
  ))
(add m notes '(
  "Mines just sit there until someone comes by, at which point they blow up."
  "While passive and unglamorous, they can rip an attack to shreds if used"
  "in a good location."
  ))
(add ms notes '(
  "Minesweepers can lay and clear naval mines. If there is a narrow strait,"
  "a few mines can destroy an entire armada."
  ))

(game-module (design-notes (
  "Full transports should move more slowly."
  )))
