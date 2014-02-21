(game-module "logunit"
  (blurb "base module for log.g game")
  )

;;; This file is just types and tables, is not a complete game (see
;;; "log" for that).

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
(unit-type base (image-name "airbase") (char "/")
  (help "airbase plus port"))
(unit-type town (image-name "town20") (char "*")
  (help "smaller than a city"))
(unit-type city (image-name "city20") (char "@")
  (help "capital of a side"))
(unit-type farm (image-name "barn") (char "#") (help "produces food"))
(unit-type refinery (image-name "refinery") (char "%") (help "produces fuel"))
(unit-type ammo-factory (image-name "shell") (char "!")
  (help "produces ammo"))
(unit-type tanker (image-name "ca") (char "T")
  (help "transfers food, fuel, and ammo"))
;; Question: are roads important enough to make it all that important
;; to bother?  I've increased the range of armor, let's see if that helps.
(unit-type engineer (image-name "worker") (char "e")
  (help "builds roads"))

(define i infantry)
(define a armor)
(define f fighter)
(define b bomber)
(define d destroyer)
(define s submarine)
(define t troop-transport)
(define cv carrier)
(define bb battleship)
(define B base)
(define ~ town)
(define @ city)
(define # farm)
(define % refinery)
(define ! ammo-factory)
(define T tanker)
(define e engineer)

; The danger in having a lot of materials is that they are redundant
; with each other.  It doesn't provide any challenge, just tedium.
; My main goal in having as many as we do is to slow expansion.
(material-type fuel
  (help "basic supply that units need to move"))
(material-type ammo
  (help "generic supply used in combat"))
(material-type food
  (help "basic supply that all units need"))

;; I think that supply-on-creation defaults to zero, which is
;; what I want.

(include "stdterr")

(define ground (i a engineer))
(define aircraft (f b))
(define ship (d s t cv bb tanker))
(define cities (~ @))
(define places (B ~ @))
(define movers (i a f b d s t cv bb tanker engineer))

(define water (sea shallows))
(define land (swamp plains forest desert mountains))

;;; Static relationships.

(table vanishes-on
  (ground water true)
  (armor swamp true)
  (places water true)
  ;; We'll allow farms, refineries, or ammo-factories on most terrain
  ;; albeit with productivity differences.
  ((farm refinery ammo-factory) water true)
  (ship land true)
  (ship road true)
  ;; Only aircraft can deal with ice.
  (u* ice true)
  (aircraft ice false)
  )

;; Unit-unit capacities.

;; Note that carriers have room for up to 8 fighters, but only
;; two bombers.

(table unit-capacity-x
  (cv fighter 4)
  )

(add b capacity 1)
(add t capacity 6)
(add cv capacity 4)
(add places capacity (20 40 80))
(add (farm refinery ammo-factory) capacity 5)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  ;; Transports can carry 6 armor, infantry, or engineers.
  (ground t 1)
  ((f b) cv (1 2))
  ;; Bases can hold 10 of most unit types, but up to 20 fighters.
  (u* B 2)
  (fighter B 1)
  ;; City types have lots of capacity for everything.
  (movers cities 1)
  ;; Need to hold a moderate number of units or else it would be too
  ;; hard to move through them to get somewhere.  Aircraft fly over.
  ;; To build a canal or otherwise accomodate ships, you need to
  ;; build bases instead.
  ;; Maybe should use the 13/16 size as in empire.g?
  ;; Problem with allowing occupants is that the parent may shield its
  ;; occupants from attack too much.
  (ground (farm refinery ammo-factory) 1)
  )

;;; Unit-terrain capacities.

;; Limit units to 16 in one cell, for the sake of playability and
;; and drawability.  Places cover the entire cell, however.
  
(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  ;; Allow aircraft to pass over towns instead of landing.
  (aircraft t* 0)
  (places t* 16)
  ((farm refinery ammo-factory) t* 16)
  )

;;; Unit-material capacities.

;; Cities and towns store less than the standard game.  This is so that
;; capturing the enemy's towns won't solve your material problems quite
;; as much.

;; Armor stores more than the standard game, otherwise one of its big
;; advantages over infantry (rapid movement) is diminished.
(table unit-storage-x
  ;; this determines how long a unit can survive on its own
  (u* food ( 6 10  6 18 100 100 200 400 200  200 200 400 200  0   0 500 6))
  ;; this one is important for movers; it sets the range.
  (u* fuel ( 6 20 18 36 100 100 200 400 200  200 200 400  0 200   0 500 6))
  (u* ammo ( 6  4  3  3  20  10  20  40  40  100 100 200  0   0  60 200 0))
  )

;;; Vision.

;; Towns and cities always have foreign correspondents, telephones,
;; private citizens coming and going, so their state is always
;; going to be available to any side that knows they exist.

(add cities see-always true)

;; Cities have more powerful radar and sensing systems, and
;; so they can see out further.

(add @ vision-range 3)
;(add bb vision-range 2)  ; an experiment

(table see-chance-adjacent
  ;; Submarines are always hard to see.
  (u* s 10)
  )

;;; Actions.

(add u* acp-per-turn  (1 2 9 6 3 3 2 4 4 0 1 1 0 0 0 2 1))

;;; Movement.

(add places speed 0)
(add (farm refinery ammo-factory) speed 0)

;; Don't be too picky about mp usage.

(add u* free-mp 1)

(add aircraft free-mp (9 6))

(table mp-to-enter-terrain
  ((cv bb) shallows 2)
  (a (swamp forest mountains) 99)
  ;; No special trouble to get on a road
  ;; (such as when using to cross a river).
  (ground road 0)
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
  ;; In this game, armor and infantry can only cross a river if
  ;; engineers build a bridge
  (u* river 0)
  ((i a) river 99)
  )

(table mp-to-leave-terrain
  ;; This is for accident prevention, in the case of a ground
  ;; unit on a road bridge that doesn't quite reach land.
  (ground water 99)
  ;; Prevent armor crossing impassable terrain from a road
  ;; ending in impassable terrain.
  (a (swamp forest mountains) 99)
  )

(table mp-to-traverse
  (a road 1)
  (ship river 2)
  )

(table material-to-move
  (movers fuel 1)
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
  )

;;; Construction.

;; Basically, units take about as many turns to complete as their cp.

(add u* cp (4 7 8 16 10 16 12 30 40 5 1 1 9 9 9 16 10))

(table can-create
  (i (B farm refinery ammo-factory) 1)
  (cities movers 1)
  )

(table acp-to-create
  (i (B farm refinery ammo-factory) 1)
  (cities movers 1)
  )

(table cp-on-creation
  (a B 2)
  )

(table can-build
  (i (B farm refinery ammo-factory) 1)
  (cities movers 1)
  )

(table acp-to-build
  (i (B farm refinery ammo-factory) 1)
  (cities movers 1)
  )

(table cp-per-build
  (a B 2)
  )

(table occupant-can-construct (u* u* false))

;;; Repair.

;; Explicit repair actions accelerate the recovery of lost hp.

(table can-repair
  (cities u* true)
;  (B u* true)
  (cv cv true)
  (bb bb true)
  (i places true)
  )

(table acp-to-repair
  (cities u* 1)
;  (B u* 1)
  (cv cv 1)
  (bb bb 1)
  (i places 1)
  )

(table hp-per-repair
  ;; Towns and cities can repair anything.
  (cities u* 1.00)
;  (B u* 3.00)  ; what is this all about?
  ;; Capital ships are equipped to do major repairs to themselves.
  (cv cv 1.00)
  (bb bb 1.00)
  ;; The army's engineers can do lots of repair work if put to the task.
  (i places (2.00 5.00 5.00))
  )

(table auto-repair
  ;; Capital ships are equipped to do major repairs to themselves.
  (cv cv 0.50)
  (bb bb 0.50)
  (cities movers 0.50)
  )

;; Some types have sufficient people and redundancy to do some repair work
;; without affecting their readiness to act.

(add (cv bb B) hp-recovery 0.50)
(add cities hp-recovery 1.00)
(add (farm refinery ammo-factory) hp-recovery 1.00)

;; Terrain alteration.  The idea behind having a separate (expensive)
;; engineer type and the cost is to make it hard to build roads.
;; I'd kind of like to have a road take multiple turns to build but
;; I don't think xconq can do that.
(table acp-to-add-terrain
  (engineer road 1)
  )
;; But the engineers need the ability to store that material.
;; and do we want to extend their range that much?
;(table material-to-add-terrain
;  (engineer fuel 50)
;  )

;;; Production.

(table base-production
  ; Cities, towns, and bases just transfer supplies, don't produce them.
  (farm food 100)
  (refinery fuel 100)
  (ammo-factory ammo 30)
  )

(table productivity
  (B land (0 100 50 20 20))
  (~ land (0 100 50 20 20))
  (@ land (0 100 50 20 20))
  (farm land (0 100 50 20 20))
  (refinery land (0 100 50 20 20))
  (ammo-factory land (0 100 50 20 20))
  )

;; Making it so that farms, refineries and ammo-factories do not
;; consume food keeps life simple - that way they don't need any
;; storage or in-length on food, which in turn prevents them from
;; being supply depots (albeit minor ones).
(table base-consumption
  ((f b) fuel (3 2))
  (movers food 1)
  )

(table hp-per-starve
  ((i f b) fuel 1.00)
  ;; Immobilized tanks eventually rust...
  (armor fuel 0.05)
  (ship fuel 0.10)
  (places fuel 0.05)

  ((i f b) food 1.00)
  ;; Immobilized tanks eventually rust...
  (armor food 0.05)
  (ship food 0.10)
  (places food 0.05)
  )

(table consumption-as-occupant
  ;; Aircraft on the ground or in a carrier just sit there.
  ((f b) fuel 0)
  )

;;; Combat.

(add u* hp-max (1 1 1 2 3 2 3 4 8 10 20 40 20 20 20 2 1))

;; Units are generally crippled, moving at half speed,
;; at about 1/2 of hp-max, sometimes rounding up, sometimes down.

(add b  speed-damage-effect ((1 50) (2 100)))
(add d  speed-damage-effect ((1 50) (2 100) (3 100)))
(add s  speed-damage-effect ((1 50) (2 100)))
(add t  speed-damage-effect ((1 50) (2  50) (3 100)))
(add cv speed-damage-effect ((1 50) (2  50) (3 100) (4 100)))
(add bb speed-damage-effect ((1 50) (4  50) (5 100) (8 100)))
(add T  speed-damage-effect ((1 50) (2  50) (3 100)))

(table acp-to-attack
  ;; Places can defend, but not attack.
  (places u* 0)
  )

;;; The main hit table.

(table hit-chance
;          i   a   f   b   d   s   t  cv  bb    B   ~   @  #  %  !  T  e
  (i u* ( 50  40  20  15  20  20  30  20   9   80  60  40 10 10 10 20 50))
  (a u* ( 60  50  30  30  30  20  30  20  20   90  70  50 20 20 20 30 50))
  (f u* ( 15  25  60  70  20  30  20  50  40  100 100 100 15 15 15 20 15))
  (b u* ( 20  20  10   9  30  50  50  70  60   90  95  99 20 20 20 30 30))
  (d u* (  5   5  10   5  60  70  60  40  20   99  90  80  5  5  5 60  5))
  (s u* (  0   0  10   5  40  10  60  40  50    0   0   0  5  5  5 60  5))
  (t u* ( 20   5  10   5  40  40  40  30   9    0   0   0  0  0  0 30  0))
 (cv u* ( 30  20  40  10  30  30  40  20  20    0   0   0  0  0  0 30  0))
 (bb u* ( 50  50  50  20  70  50  90  50  90  100 100 100 40 40 40 80 50))
  (B u* ( 10  10  20  20  20  20  30  20  20    0   0   0  0  0  0 20 10))
  (~ u* ( 30  20  50  40  40   0  30  20  20    0   0   0  0  0  0 30 20))
  (@ u* ( 50  40  70  60  50   0  30  20  50    0   0   0  0  0  0 40 30))
  (# u* ( 10  10  20  20  20  20  30  20  20    0   0   0  0  0  0 20 10))
  (% u* ( 10  10  20  20  20  20  30  20  20    0   0   0  0  0  0 20 10))
  (! u* ( 10  10  20  20  20  20  30  20  20    0   0   0  0  0  0 20 10))
  (T u* ( 20   5  10   5  40  40  40  30   9    0   0   0  0  0  0 30  5))
  (e u* ( 50  40  20  15  20  20  30  20   9   10  10  10 10 10 10 20 50))
  )

(table damage
  (u* u* 1)
  (a places 2)
  (b ship 2)
  (b s 1)
  (b places (2 2 3))
  (d s 2)
  (s ship 3)
  (s bb 4)
  (bb u* 2)
  (bb cities (3 4))
  )

;; Note that I do not allow capturing farms, refineries, and ammo-factories.
;; This is because if you can, then starting to take over the enemy's
;; territory solves your logistics problems.  It's an easy way out.
;; Similarly, they are intended to be relatively difficult to destroy
;; although I'm not sure I have that balance quite right.
(table capture-chance
  (i places (70 50 30))
  (a places (90 70 50))
  )

(table independent-capture-chance
  (i places (100 80 50))
  (a places (100 95 70))
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
  (places movers 50)
  ;; but bases make aircraft sitting ducks, so no protection there.
  (base aircraft 100)
  ;; inf and armor protect the places housing them.
  ;; can't make this too large or city can be
  ;; invulnerable.
  (i places 80)
  (a places 60)
  ; bases benefit more from protection.
  ((i a) B (50 40))
  )

;; Combat requires ammo, and uses it up.

(table consumption-per-attack (u* ammo 1))

(table hit-by (u* ammo 1))

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
  ((i a b f) m* -1)
  ;; I thought about (tanker m* 3) and likewise for in-length, the idea
  ;; being that you'd build whole fleets of them, but that is probably
  ;; a bit excessive.
  (tanker m* 6)
  ;; Cities, towns, and bases are good at sharing things around.
  (B m* 6)
  ;; In the standard game this is 12, but the problem is that if you are
  ;; capturing the enemy's towns, there is a good chance the one you
  ;; captured will be within 12 hexes of one of your towns.  I'd rather
  ;; make you use a tanker or build some bases or something (this ~is*
  ;; a logistics game).
  (cities m* 6)
  (farm food 6)
  (refinery fuel 6)
  (ammo-factory ammo 6)
  )

(table in-length
  ;; Supply to ground units can go a couple hexes away.
  (ground m* 3)
  (tanker m* 6)
  ;; Cities and bases can get their supplies from some distance away.
  (B m* 6)
  (cities m* 6)
  )

;; Fate of units when a side loses.

;; Towns and cities are forever.
(add B lost-vanish-chance 1000)
(add cities lost-vanish-chance 0)

(table lost-surrender-chance (u* u* 5000))

(add B lost-revolt-chance 10000)
(add cities lost-revolt-chance 10000)

;;; Scoring.

;; Most units aren't worth much, but ground units can capture
;; cities, carriers and battleships are powerful.

(add u* point-value 0)
(add (i a) point-value 1)
(add (cv bb) point-value 1)
(add cities point-value (5 25))

;;; Texts.

(set action-notices '(
  ((disband infantry self done) (actor " goes home"))
  ((disband u* u* done) (actee " disbands"))
  ((destroy u* ground) (actor " defeats " actee "!"))
  ((destroy u* aircraft) (actor " shoots down " actee "!"))
  ((destroy u* ship) (actor " sinks " actee "!"))
  ((destroy u* places) (actor " flattens " actee "!"))
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

(add cities start-with (5 1))
;; This is a kludge whose purpose is to make the AI
;; work.  The AI isn't clever enough to build ammo-factories,
;; farms, and refineries, at least in the game I tried.
;; Even with bases in the standard game, it is odd.
(add farm start-with 1)
(add refinery start-with 1)
(add ammo-factory start-with 1)

(set country-radius-min 3)
(set country-separation-min 16)
(set country-separation-max 48)
;; Try to get countries to be on the coast.
(add (sea plains) country-terrain-min (1 4))

(table favored-terrain
  (u* t* 0)
  (@ land 20)
  (~ land 20)
;  (~ plains 40)
  (farm plains 100)
  (refinery plains 100)
  (ammo-factory plains 100)
  )

(table independent-density
  ;; Most additional towns are in favorable terrain.
  (town (plains desert forest mountains) 250)
  )

(add land country-people-chance 90)
(add plains country-people-chance 100)

(add land independent-people-chance 50)

;; Now that engineers can build roads, might want to tweak these
;; so that there are fewer roads at the start of the game.  I'll
;; leave it as is for the moment.
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

;; Give the player enough supplies to last until they can build their
;; farms and refineries.
;; I'd like to make them build all their own ammo, but the handling of
;; supply low conditions is just too annoying.  There's also the
;; question of making them build an ammo factory before they capture
;; independent towns, but I think that is a feature, not a bug
;; (slightly less of a windfall to start near an independent town, for
;; one thing).
(table unit-initial-supply
  (u* fuel 200)
  (u* food 200)
  (u* ammo 50))

;; Default doctrine.

(doctrine default-doctrine
  (construction-run (u* 1))
  (rearm-percent 40)
  )

(doctrine place-doctrine
  (construction-run (u* 99) ((carrier battleship) 3)))

(side-defaults
  (default-doctrine default-doctrine)
  (doctrines (places place-doctrine))
  )

(game-module (notes (
  "This game is based on the standard game, but requires more attention"
  "to logistics."
;  ""
;  "Opinions differ about optimal strategy for this game.  In general,"
;  "blitzkrieg works, and can win the game in a hurry.  The problem is to"
;  "muster enough force before striking.  One full troop transport is not"
;  "enough; the invasion will melt away like ice cream on a hot sidewalk,"
;  "unless"
;  "reinforcements (either air or land) show up quickly.  Air cover is very"
;  "important.  While building up an invasion force, airborne assaults using"
;  "bombers and infantry can provide useful diversions, although it can be"
;  "wasteful of bombers.  Human vs human games on a 60x30 world generally"
;  "last about 100 turns, usually not enough time or units to build atomic"
;  "bombs or battleships, and not a big enough world to really need carriers,"
;  "although bases for staging are quite useful."
  )))

(add i notes '(
  "The infantry army is the slowest of units, but it can go almost"
  "anywhere.  It is also quick to produce.  Infantry is the staple of"
  "campaigns - no special features, but essential to success."
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
  "Their fuel supply can be gotten only at cities/towns, bases,"
  "and carriers, so they must continually be taking off and landing."
  "Fighters are not too effective"
  "against ground units or ships, but they eat bombers for lunch.  Fighters"
  "are very good for reconnaisance - important when you can't always"
  "see the enemy moving!"
  ))
(add b notes '(
  "Bombers are very powerful, since they can seriously damage"
  "or even flatten cities.  Loss rate in such activities is high,"
  "so they're not a shortcut to victory!"
  "Bombers can carry one infantry, which is very useful for raids."
  ))
(add d notes '(
  "Destroyers are fast small ships for both exploration and"
  "anti-submarine activities."
  ))
(add s notes '(
  "The favorite food of a submarine is a troop transport, which it can"
  "sink with one blow."
  "Subs are also hard to spot, but vulnerable to destroyers and aircraft."
  ))
(add t notes '(
  "This is how ground units get across the sea.  Transports can"
  "defend themselves against ships and aircraft, but are basically vulnerable."
  "They're not very fast either."
  ))
(add cv notes '(
  "Compensates for the limited range of fighters and bombers by providing"
  "a portable airport.  Carriers themselves are sitting ducks, particularly"
  "with respect to aircraft.  Fighter patrols are mandatory."
  ))
(add bb notes '(
  "The aptly named `Dread Naught' has little to fear from other"
  "units of this period.  Subs may sink them with enough effort, and a group"
  "of bombers and fighters are also deadly,"
  "but with eight hit points to start,"
  "a battleship can usually survive long enough to escape."
  "Battleships are very effective against cities and armies,"
  "at least the ones on the coast."
  ))
(add B notes '(
  "A base serves as a combination camp, airbase, and port."
  "Bases cannot build units, although they can repair some damage."
  ))
(add ~ notes '(
  "Towns are the staple of territory.  They can build, repair,"
  "and serve as a safe haven for quite a few units."
  ))
(add @ notes '(
  "Cities are very large, powerful, and well defended.  They are"
  "basically capital cities, or something in a comparable range.  (New York"
  "and San Francisco are cities, Salt Lake City and San Antonio are towns."
  "Yeah, San Antonio has a lot of people, but it's still insignificant,"
  "nyah nyah.)  A city is worth five towns, point-wise."
  ))
(add farm notes '(
  "Fail to have enough farms and your units will starve."
  "You'll want to start the game by building at least one."
  ))
(add refinery notes '(
  "Units need fuel to move, so building at least one refinery"
  "is a high priority."
  ))
(add ammo-factory notes '(
  "An ammo factory builds the ammunition which you need to attack"
  "(including to capture independent towns)."
  ))
(add tanker notes '(
  "This lets you transfer supplies across the sea where the distance"
  "is too great to just do it between cities, towns and bases directly."
  "Can make the initial stages of exploring or colonizing another island"
  "much more practical."
  ))
(add engineer notes '(
  "The engineer has only one trick, but it can be a very useful one, and"
  "that is to construct roads.  Especially over rivers, which are otherwise"
  "impassable by infantry and armor."
  ))

;(set unseen-color "orange")
;(set grid-color "black")
