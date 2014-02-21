(game-module "ww2-div-eur"
  (title "WWII Division Level Land")
  (blurb "Base module for division level land war. Emphasis on Europe.")
  (version "1.0")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
   ("Last Side Wins" keep-score true (true (scorekeeper (do last-side-wins))))
   )
  )

(unit-type a-inf (name "infantry(a)") (image-name "inf-div")
  (help "Allied infantry division"))
(unit-type a-armor (name "armor(a)") (image-name "armor-div")
  (help "Allied armored division"))
(unit-type a-para (name "paratroops(a)") (image-name "para-div")
  (help "Allied paratroop division"))

(define a-ground-types (a-inf a-armor a-para))

(add a-ground-types possible-sides "allies")

(add a-ground-types acp-per-turn (3 6 3))

(unit-type g-inf (name "infantry(g)") (image-name "inf-div")
  (help "German infantry division"))
(unit-type g-armor (name "armor(g)") (image-name "armor-div")
  (help "German Panzer division, fast and dangerous"))
(unit-type g-para (name "paratroops(g)") (image-name "para-div")
  (help "German paratroop division"))
(unit-type pzgren (name "panzergrenadiers") (image-name "pzgren-div")
  (help "German panzergrenadier division"))
(unit-type metro (image-name "metro-div")
  (help "German metropolitan defense division"))
(unit-type cd (name "coastal defense") (image-name "unit")
  (help "German coastal defense division"))

(define g-ground-types (g-inf g-armor g-para pzgren metro cd))

(add g-ground-types possible-sides "axis")

(add g-ground-types acp-per-turn (3 5 3 3 3 2))

(unit-type hq (image-name "army-hq")
  (help "Army HQ, one needed for every 16 divisions"))

(add hq acp-per-turn 6)

(define ground-types (append a-ground-types g-ground-types hq))

(define non-mot-types (a-inf g-inf))

(define mot-types (a-armor g-armor))

(unit-type train
  (help "rail transport for speedy movement behind the lines"))

(add train acp-per-turn 28)

;; should have assault craft distinct from transports.(?)
(unit-type transport (image-name "ap")
  (help "sea transport for ground units"))
(unit-type battleship (image-name "bb")
  (help "shore bombardment capability"))

(define ship-types (transport battleship))

(add ship-types acp-per-turn 140)

(unit-type bomber (image-name "4e")
  (help "tactical bombing capability, use against ground units"))
(unit-type v2-launcher (image-name "v2")
  (help "V-2 rocket launcher, use against cities and airbases"))

(add bomber acp-per-turn 2)
(add v2-launcher acp-per-turn 8)

(unit-type airbase
  (help "homes for bombers"))
(unit-type port (image-name "anchor")
  (help "supply sources for invading forces"))
(unit-type fortification (image-name "fort")
  (help "improved positions"))
(unit-type town (image-name "town20")
  (help "small or medium-sized town, minimal defensive effect"))
(unit-type city (image-name "city19")
  (help "large city, very good for defense"))

(define place-types (airbase port fortification town city))

(add place-types acp-per-turn 0)

(material-type supply
  (help "general supply - important to do things, acquired from HQs"))
(material-type missile
  (help "what the V-2 launchers shoot with"))

; sea clr brkn/bocage rough/hills mtn urb estuary road rail river rhine

(terrain-type sea (char ".")
  (help "the high seas, and coastal waters"))
(terrain-type swamp (char "=")
  (help "soggy and overgrown terrain, difficult for vehicles"))
(terrain-type lowland (image-name "flooded")
  (help "near-sea-level terrain, flat but channeled access"))
(terrain-type clear (image-name "plains") (char "+")
  (help "open and unobstructed, good for armor"))
(terrain-type broken (image-name "bocage") (char "%")
  (help "rolling and hedgerow countryside"))
(terrain-type hills (image-name "forest") (char "^")
  (help "rugged and/or wooded hilly areas"))
(terrain-type mountains (char "/")
  (help "extremely rugged terrain, difficult access"))
(terrain-type urban (char "#")
  (help "built-up areas around major towns and cities"))
(terrain-type estuary (image-name "shallows")
  (help "shallow restricted waters where rivers meet the sea"))
(terrain-type river
  (subtype border) (subtype-x river-x)
  )
(terrain-type Rhine-river (image-name "river")
  (subtype border)
  )
(terrain-type beach
  (subtype border) (subtype-x river-x)
  )
(terrain-type road
  (subtype connection) (subtype-x road-x)
  )
(terrain-type railroad
  (subtype connection) (subtype-x road-x)
  (help "where trains can go"))

(define water-t* (sea estuary))

(define land-t* (swamp lowland clear broken hills mountains urban))

(define cell-t* (sea swamp lowland clear broken hills mountains urban estuary))

(add water-t* liquid true)

;;; Static relationships.

(table vanishes-on
  (ground-types water-t* true)
  (ship-types land-t* true)
  (v2-launcher water-t* true)
  (place-types water-t* true)
  )

;; Unit-in-terrain capacities.

(table unit-size-in-terrain
  ;; Stacking limits are for military units only.
  (place-types t* 0)
  )

(add t* capacity 3)

;; Unit-in-unit capacities.

(table unit-size-as-occupant
  (u* u* 99)
  (ground-types (transport city) 1)
  ;; Trains can carry one unit each.
  (ground-types train 1)
  (train city 1)
  )

(add transport capacity 4)
(add train capacity 1)
; Note that towns are too small to contain any units.
(add city capacity 6)

(table unit-capacity-x
  (port ship-types (6 2))
  (airbase bomber 2)
  )

;; Unit-material capacities.

(table unit-storage-x
  (ground-types supply 4)
  (hq supply 20)
  (port supply 100)
  (v2-launcher missile 14)
  )

;;; Vision.

;;; Static units can't do anything without the world's reporters finding
;;; out within the week.

(add place-types see-always true)

;;; Actions.


(table material-to-act
  (ground-types supply 1)
  )

;;; Movement.

(add place-types speed 0)
(add bomber speed 0)

(table mp-to-enter-terrain
  (mot-types hills 2)
  (ground-types mountains 2)
  (mot-types mountains 4)
  (hq mountains 1)
  (a-ground-types clear 1)
;  (a-ground-types sea (3 6 3))
  (ground-types river 2)
  (ground-types Rhine-river 3)
  (ground-types water-t* 9999)
  ;; Trains can only move on railroads.
  (train t* 9999)
  (train railroad 0)
  (ship-types land-t* 9999)
  (a-ground-types beach -90)
  )

(table mp-to-leave-terrain
  (a-ground-types sea 90)
  ;; Trains can only move on railroads.
  (train t* 9999)
  (train railroad 0)
  )

(table mp-to-traverse
  (ground-types road 1)
  (train railroad 1)
  )

(table ferry-on-departure
  (transport ground-types over-nothing)
  )

;; Zones of control.

(table zoc-range
  ;; Ground units can constrain the movement of adjacent ground units.
  (ground-types ground-types 1)
  ;; Except for HQs, which are wimpy combatwise.
  (hq u* 0)
  ;; Ground types can also control estuaries.
  (ground-types ship-types 1)
  )

(table zoc-into-terrain
  (ground-types t* false)
  (ground-types land-t* true)
  (ground-types estuary true)
  )

(table mp-to-enter-zoc
  (ground-types ground-types 7)
  (ship-types ground-types 980)
  )

(table mp-to-traverse-zoc
  ;; No shifting across an enemy ZOC.
  (ground-types ground-types -1)
  (ship-types ground-types 980)
  )

(table mp-to-leave-zoc
  (ground-types ground-types 2)
  (ship-types ground-types 980)
  )

;;; Repair.

;; (should only happen to supplied units)

(add u* hp-recovery 0.50)

;;; Combat.

(add ground-types hp-max 12)
(add ship-types hp-max 24)
(add (bomber v2-launcher) hp-max 12)
(add place-types hp-max 48)

(table acp-to-attack
  (u* u* 1)
  ;; Metro divisions are purely defensive.
  (metro u* 0)
  ;; Similarly for HQ units.
  (hq u* 0)
  ;; Ships can only shell things.
  (ship-types u* 0)
  ;; Transportation types can't actually attack anything.
  (train u* 0)
  ;; V-2 bases can only fire missiles.
  (v2-launcher u* 0)
  )

(table hit-chance
  (u* u* 50)
  (a-inf u* 60)
  (a-armor u* 40)
  (a-para u* 50)
  (g-inf u* 40)
  (g-armor u* 40)
  (g-para u* 70)
  (pzgren u* 60)
  (cd u* 30)
  ;; HQs are not for combat.
  (hq u* 0)
  ;; Transport combat is non-existent at this scale.
  (u* transport 0)
  (transport u* 0)
  ;; Battleships are very good at shelling everything.
  (battleship u* 90)
  ;; ...but not so good with each other.
  (battleship battleship 50)
  ;; Might accidentally destroy what we're trying to capture.
  (u* train 10)
  ;; Bombers can't hit V-2 bases, they're too small and mobile.
  (bomber v2-launcher 0)
  ;; V-2 bases can only hit stationary targets.
  (v2-launcher u* 0)
  (v2-launcher place-types 50)
  )

(table damage
  (u* u* 2d4)
  (hq u* 0)
  (u* transport 0)
  (transport u* 0)
  (u* train 1)
  ;; V-2 bases can only hit stationary targets.
  (v2-launcher u* 0)
  (v2-launcher place-types 2d4)
  )

(table hp-min
  ;; Places can never be totally destroyed.
  ;; (then what is effect of damage on a place??)
  (u* place-types 1)
  ;; Bombers are not capable of destroying ground units.
  (bomber ground-types 6)
  ;; Neither are ships.
  (ship-types ground-types 6)
  )

(table withdraw-chance-per-attack
  ;; HQs are supposed to preserve themselves.
  (u* hq 90)
  )

(table stack-protection
  ;; Westwall gives a considerable bonus to German troops on it.
  (fortification g-ground-types 50)
  ;; Can't capture any place if enemy units are stacked with it.
  (ground-types place-types 0)
  (ground-types v2-launcher 0)
  )

;; Battleships can reach inland up to 20 miles.

(add battleship acp-to-fire 70)
(add battleship range 2)

;; Aircraft give cities and bases a long reach.

(add bomber acp-to-fire 1)

(add v2-launcher acp-to-fire 1)

;; Bombers can reach pretty much everywhere.

(add bomber range 60)

;; Nominally, the V-2's range was 200 miles, but be generous.

(add v2-launcher range 24)

(table consumption-per-fire
  (v2-launcher missile 1)
  )

(table hit-by
  (u* missile 1)
  )

(table capture-chance
  ;; Trains are easily captured, but might be able to escape sometimes.
  (ground-types train 80)
  ;; Places don't usually have much integral defense.
  (ground-types v2-launcher 90)
  (ground-types place-types 90)
  ;; Towns especially, you can just walk into if nobody is around.
  (ground-types town 100)
  )

;; (units in towns and trains should be able to help defend)

;;; Backdrop.

(table base-production
;  (hq supply 16)
  (port supply 16)
  (v2-launcher missile 2)
  )

(table base-consumption
  (ground-types supply 1)
  ;; HQ is automatically organized.
  (hq supply 0)
  )

(table out-length
  (hq supply 6)
  (port supply 6)
  )

(table in-length
  (ground-types supply 6)
  )

;;; Scoring.

(add u* point-value 0)
(add (town port city) point-value 1)

;;; Definitions used for testing.

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-size 20)
(set alt-smoothing 6)
(set wet-blob-density 2000)
(set wet-blob-size 100)

; sea swamp low clr brkn rough mtn urb

(add cell-t* alt-percentile-min (  0  30  32  30  30  70  90  30   0))
(add cell-t* alt-percentile-max ( 30  33  35  70  70  90 100  70   0))
(add cell-t* wet-percentile-min (  0  50  50   0  50   0   0  45   0))
(add cell-t* wet-percentile-max (100 100 100  45 100 100 100  50   0))

;;; River generation.

(add (clear hills mountains) river-chance (2.00 2.00 4.00))

(set river-sink-terrain sea)

;;; Road generation.

(table road-chance
  ((town city) (town city) 100)
  ((town city) (port airbase) 100)
  )

(table road-into-chance
  (land-t* land-t* 100)
  )

(set country-radius-min 6)

(add (sea clear) country-terrain-min (5 5))

(add u* start-with 1)  ; one of everything

(table favored-terrain
  (u* t* 100)
  (u* (sea swamp) 0)
  (ship-types sea 100)
  (ship-types land-t* 0)
  )

(table unit-initial-supply
  ;; Everybody starts out fully supplied.
  (ground-types supply 999)
  (v2-launcher missile 14)
  )

;;; No political detailing needed at the divisional level.

(set side-library '(
  ((name "Allies") (adjective "Allied") (class "allies")
   (emblem-name "white-star"))
  ((name "Axis") (adjective "Axis") (class "axis")
   (emblem-name "german-cross"))
  ))

(world 2500 (year-length 52))

(set calendar '(usual week))

(game-module (design-notes (
  "The map scale is 10 miles/hex, game time is 1 week/turn."
  ""
  "This game design covers division-level operations in Europe during WWII."
  "Players take on the roles of army commanders but not political leaders;"
  "the game does not include political factors."
  )))

(add a-inf notes '(
  "Allied infantry is slow but steady.  It can be carried by transports,"
  "but can't unload just anywhere; it must get off either across a beach"
  "or in a port."
  ))

(add a-armor notes '(
  "The Allied armor division dashes about and hits hard; every energetic"
  "commander wants more.  Alas, very few are available, and to lose one"
  "would be a disaster.  Armor can also easily outstrip HQs and"
  "come to a halt for lack of supply."
  ))

(add a-para notes '(
  "The Allied paratroopers (82nd and 101st American, 1st and 6th British),"
  "are the elite of their nations' armed forces.  However, they are"
  "still basically foot soldiers, and so not especially fast, and their"
  "artillery support is less than what the regular infantry gets, and"
  "so their combat ability is less."
  ))

(add g-inf notes '(
  "The German infantry is tough and experienced, but later in the"
  "wars, losses and limited manpower reduce their effectivness,"
  "and so they are weaker than their Allied counterparts."
  ))

(add g-armor notes '(
  "German armor offers speed only.  Its smaller size and relative"
  "paucity of artillery means that it has no more combat effectiveness"
  "than infantry.  But moving at nearly twice the speed, armor"
  "is useful for plugging holes in the line."
  ))

(add g-para notes '(
  "The German paratroops are the best-equipped and -led divisions,"
  "and are thus the strongest in combat."
  ))

(add pzgren notes '(
  "Unlike the other German ground units (aside from armor), the"
  "Panzergrenadiers have their own motorized transport."
  ))

(add cd notes '(
  "The coastal defense divisions illustrate Germany's desperate"
  "situation by 1944.  They are partly composed of impressed soldiers"
  "from conquered countries, they have little transport, and are"
  "not much trained.  Their movement and combat ability are less"
  "than that of any other type of unit in this game.  However,"
  "the Allied player should not discount them!  They can still"
  "block the path, particularly at critical moments giving the"
  "infantry and armored divisions time to come to the rescue."
  ))

(add v2-launcher notes '(
  "The fearsome V-2 rocket makes its first appearance in the fall of 1944."
  "A ballistic missile with a 200-mile range, there is no way to defend"
  "any unit within the range of a mobile launcher.  The apparatus is"
  "easily wrecked or captured, so the Germans need to keep their"
  "launchers safely back from the front lines, and to move them around"
  "so as to avoid being targeted by Allied bombers.  Launchers"
  "`produce' their own missiles, but only a couple each turn, which"
  "limits the number of attacks possible."
  ))

#|

Parachutists have restrictions on initial placement (why?), won't
drop further than 10 hexes from friendlies (even on invasion?)
(use control radius to impl)

Can build fortresses (very slowly), units can be inside or outside.

Admin movement should also use trucks.

All units have medium prob to surrender - use ZOC to calculate.

Add air transport for paratroops to ride in.

|#
