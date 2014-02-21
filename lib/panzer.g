(game-module "panzer"
  (title "Panzer")
  (version "1.0")
  (blurb "Tactical-level WWII on the Eastern Front.")
  (picture-name "panzer")
  (variants
   (world-seen true)
   (see-all false)
   (sequential false)
   (world-size (80 40 25000))
   (real-time (0 0 0))
   ("Capture the Town" capture-the-town
    "Each side's goal is to capture the other side's town."
    (true
      (add town start-with 1)
      (add u* point-value 0)
      (add town point-value 1)
      ))
   ("Minefields" minefields
    "Minefields are present."
    (true
      ;; if this is enabled, the AI gets confused...
	  (table independent-density
	    (minefield clear 500)
	    (minefield woods 100)
	    )
      ))
   ("Rougher" rougher
    "Terrain is more hilly, harder to get a clear line of sight."
    (true
	  (add t* elevation-max 1000)
      ))
   ("Night" night
    "Everything is dark, visibility is reduced, etc."
    (true
	  (world (day-length 144))
      (set initial-day-part 72) ; midnight
      ))
   ("Alternate economy" model1
    "Use a different algorithm for moving materials among units."
    (true (set backdrop-model 1)))
   ("AltEcon uses doctrine" model1doctrine
    "When using alternate economy, don't drain units below doctrine levels."
    (true (set backdrop-ignore-doctrine 0))
    (false (set backdrop-ignore-doctrine 1)))
   )
)

(terrain-type water (image-name "water") (char ".")
  (help "open water"))
(terrain-type swamp (image-name "flooded") (char "=")
  (help "water, mud and thick vegetation - nearly impassable"))
(terrain-type gully (image-name "swamp")(char "&")
  (help ""))
(terrain-type woods (image-name "adv-forest") (char "%")
  (help "hinders visibility and combat"))
(terrain-type slope (image-name "hills") (char "/")
  (help ""))
(terrain-type clear (image-name "adv-plain") (char "+")
  (help "open flat ground"))
(terrain-type thick-forest (image-name "forest")
  (subtype border)
  (help "impassable to both wheeled and tracked vehicles"))
(terrain-type stream (image-name "river-conn")
  (subtype connection) (subtype-x river-x)
  (help ""))
(terrain-type road
  (subtype connection) (subtype-x road-x)
  (help "faster travel for vehicles"))

(add (water swamp) liquid true)

(define cell-t* (water swamp gully woods slope clear))

;;; Ground is what doesn't require swimming.

(define ground (swamp gully clear woods slope))

;;; Vehicles require solid ground.

(define solid (gully clear woods slope))

;;; Wheeled vehicles can't deal with woods.

(define drivable (gully clear slope))

;; Distances are in meters.

(add t* elevation-min 100)
(add t* elevation-max 300)

(area (cell-width 400))

(add clear river-chance 5.00)

(table drawable-terrain
  (t* t* true)
  (stream (water swamp) false)
)

(set no-indepside-ingame true)

;;; Definitions of all the unit types.

;;; Random gen should do these in clumps?
(unit-type town (image-name "town20"))
(unit-type block)
(unit-type minefield)
(unit-type cmdpost (name "command post") (image-name "cmd-post")
  (acp-per-turn 1))

(add (town block minefield) speed 0)

;;; Russian unit types.

(unit-type rmg (name "MG") (image-name "pz-flak")
  (help "Russian 12.7mm machine gun"))
(unit-type r45 (name "45mm AT") (image-name "pz-at")
  (help "Russian 45mm anti-tank"))
(unit-type r57 (name "57mm AT") (image-name "pz-at")
  (help "Russian 57mm anti-tank"))
(unit-type r76at (name "76mm AT") (image-name "pz-at")
  (help "Russian 76mm anti-tank"))
(unit-type r122 (name "122mm AT") (image-name "pz-at")
  (help "Russian 122mm anti-tank - has a long reach"))
(unit-type r76h (name "76mm H") (image-name "pz-how")
  (help "Russian 76mm howitzer"))
(unit-type r82m (name "82mm M (Mot)") (image-name "pz-mortar")
  (help "Russian motorized 82mm mortar"))
(unit-type r82 (name "82mm M") (image-name "pz-mortar")
  (help "Russian 82mm mortar"))
(unit-type r120 (name "120mm M(r)") (image-name "pz-mortar")
  (help "Russian 120mm mortar - a powerful long-range hitter"))

(define r-arty (rmg r45 r57 r76at r76h r122 r82m r82 r120))

(add r-arty generic-name "arty")

(add r-arty acp-per-turn 1)
(add r-arty speed 0)
(add (r82m r82) speed 2.00)

(add r-arty acp-to-fire 1)

(add r-arty range ( 6  3  4  5 10 20 12 12 20))

(unit-type rengrs (name "engineer platoon(r)") (image-name "engr"))
(unit-type rrecon (name "recon platoon") (image-name "inf-plt"))
(unit-type rrifle (name "rifle platoon(r)") (image-name "inf-plt"))
(unit-type rguard (name "guards platoon") (image-name "inf-plt"))
(unit-type rsmg (name "SMG platoon(r)") (image-name "inf-plt"))

(define r-inf (rengrs rrecon rrifle rguard rsmg))

(add r-inf generic-name "inf")

(add r-inf acp-per-turn 2)

(unit-type rcav (name "cavalry") (image-name "cavalry")
  (acp-per-turn 6))

(unit-type rwag (name "wagon(r)") (generic-name "wagon") (image-name "wagon"))
(unit-type rtruck (name "truck(r)") (generic-name "truck") (image-name "truck"))
(unit-type rhalf (name "halftrack(r)") (generic-name "halftrack") (image-name "halftrack"))

(define r-transport (rwag rtruck rhalf))

(add r-transport acp-per-turn (6 24 20))

(unit-type su-152 (name "SU-152") (image-name "su-152"))
(unit-type su-76 (name "SU-76") (image-name "su-76"))
(unit-type su-85 (name "SU-85") (image-name "su-85"))
(unit-type su-100 (name "SU-100") (image-name "su-100"))
(unit-type jsu-122 (name "JSU-122") (image-name "jsu-122"))

(define r-td (su-152 su-76 su-85 su-100 jsu-122))

(add r-td generic-name "TD")

(add r-td acp-per-turn (14 18 22 16 14))
(add r-td acp-min (-14 -18 -22 -16 -14))

(add r-td acp-to-fire  (14 18 22 16 14))
(add r-td range        (10  5  8  8 10))

(unit-type kv85 (name "KV85") (image-name "kv85")
  (help "Early heavy tank"))
(unit-type t34 (name "T-34c") (image-name "t-34c")
  (help "The most common type of Russian tank"))
(unit-type t34/85 (name "T-34/85") (image-name "t-34-85")
  (help "A better T-34"))
(unit-type js2 (name "JS II") (image-name "js-2")
  (help "Heaviest Russian tank, but slow"))

(define r-tank (kv85 t34 t34/85 js2))

(add r-tank generic-name "tank")

(add r-tank acp-per-turn (20 22 22 16))
(add r-tank acp-min (-20 -22 -22 -16))

(add r-tank acp-to-fire  (20 22 22 16))
(add r-tank range        ( 8  6  8  8))

(define r-wheeled (rwag rtruck))

(define r-tracked (append rhalf r-td r-tank))

(define r-vehicle (append r-wheeled r-tracked))

(define r-armored (append rhalf r-td r-tank))

(define russian (append r-arty r-inf rcav r-transport r-td r-tank))

(add russian possible-sides "russian")

;;; German unit types.

(unit-type g50 (name "50mm AT") (image-name "pz-at")
  (help "German 50mm anti-tank"))
(unit-type g75 (name "75mm AT") (image-name "pz-at")
  (help "German 75mm anti-tank"))
(unit-type g88 (name "88mm AT") (image-name "pz-at")
  (help "German 88mm anti-tank"))
(unit-type g20 (name "20mm F") (image-name "pz-flak")
  (help "German 20mm flak"))
(unit-type g20q (name "20mmQUAD F") (image-name "pz-flak")
  (help "German 20mm quad flak"))
(unit-type g75h (name "75mm H") (image-name "pz-how")
  (help "German 75mm howitzer"))
(unit-type g150 (name "150mm H") (image-name "pz-how")
  (help "German 150mm howitzer - hard-hitting but shortish range"))
(unit-type g81 (name "81mm M") (image-name "pz-mortar")
  (help "German 81mm mortar"))
(unit-type g120 (name "120mm M(g)") (image-name "pz-mortar")
  (help "German 120mm mortar"))

(define g-arty (g50 g75 g88 g20 g20q g75h g150 g81 g120))

(add g-arty generic-name "arty")

(add g-arty acp-per-turn 1)
(add g-arty speed 0)

(add g-arty acp-to-fire 1)
(add g-arty range ( 5  6 20 10 10 12 12 12 20))

(unit-type gengrs (name "engineer platoon(g)") (image-name "engr")
  (help "German engineer platoon"))
(unit-type gsec (name "security platoon") (image-name "inf-plt")
  (help "German security platoon"))
(unit-type grifle (name "rifle platoon(g)") (image-name "inf-plt")
  (help "German rifle platoon"))
(unit-type gsmg (name "SMG platoon(g)") (image-name "inf-plt"))

(define g-inf (gengrs gsec grifle gsmg))

(add g-inf generic-name "inf")

(add g-inf acp-per-turn 2)

(unit-type gwag (name "wagon(g)") (generic-name "wagon") (image-name "wagon"))
(unit-type gtruck (name "truck(g)") (generic-name "truck") (image-name "truck"))
(unit-type ghalf (name "halftrack(g)") (generic-name "halftrack") (image-name "halftrack"))

(define g-transport (gwag gtruck ghalf))

(add g-transport acp-per-turn (6 24 20))

(unit-type sdkfz/1 (name "SdKfz 234/1") (image-name "sdkfz-234-1")
  (help "Weak, but fastest unit in the game"))
(unit-type puma (name "Puma"))
(unit-type sdkfz/4 (name "SdKfz 234/4") (image-name "sdkfz-234-4"))

(define g-car (sdkfz/1 puma sdkfz/4))

(add g-car generic-name "car")

(add g-car acp-per-turn (32 28 28))
(add g-car acp-min (-32 -28 -28))

(add g-car acp-to-fire  (32 28 28))
(add g-car range        ( 2  6 13))

(unit-type maultier (name "Maultier")
  (help "Second-deadliest in the game, but very vulnerable"))
(unit-type wespe (name "Wespe")
  (help "Longest range in the game, but vulnerable"))
(unit-type hummel (name "Hummel")
  (help "Deadliest unit in the game, but vulnerable"))

(define g-spa (maultier wespe hummel))

(add g-spa generic-name "SPA")

(add g-spa acp-per-turn (20 16 16))
(add g-spa acp-min (-20 -16 -16))

(add g-spa acp-to-fire  (20 16 16))
(add g-spa range        (12 32 24))

(unit-type gw38 (name "Gw 38"))
(unit-type wirbelwind (name "Wirbelwind"))
(unit-type stuh42 (name "Stu H 42"))
(unit-type marder3 (name "Marder III"))
(unit-type stug3 (name "StuG III"))
(unit-type hetzer (name "Hetzer"))
(unit-type nashorn (name "Nashorn")
  (help "Longer-range TD"))
(unit-type jgdpz4 (name "Jgd Pz IV"))
(unit-type jgdpz5 (name "Jgd Pz V"))
(unit-type jgdpz6 (name "Jgd Pz VI"))

(define ww wirbelwind)

(define g-td (gw38 ww stuh42 marder3 stug3 hetzer nashorn jgdpz4 jgdpz5 jgdpz6))

(add g-td generic-name "TD")

(add g-td acp-per-turn (12 16 16 16 16 12 16 16 18 10))
(add g-td acp-min (-12 -16 -16 -16 -16 -12 -16 -16 -18 -10))

(add g-td acp-to-fire  (12 16 16 16 16 12 16 16 18 10))
(add g-td range        (12 10 12  8  8  8 20 12 12 12))

(unit-type lynx (name "Lynx")
  (help "Weak but speedy"))
(unit-type pz4 (name "PzKpfw IV") (image-name "pz-4")
  (help "Most common type of Panzer"))
(unit-type panther (name "Panther")
  (help "All-around best tank"))
(unit-type tiger1 (name "Tiger I") (image-name "tiger-1")
  (help "Powerful but slow tank"))
(unit-type tiger2 (name "Tiger II") (image-name "tiger-2"))

(define g-tank (lynx pz4 panther tiger1 tiger2))

(add g-tank generic-name "tank")

(add g-tank acp-per-turn (20 16 20 16 12))
(add g-tank acp-min (-20 -16 -20 -16 -12))

(add g-tank acp-to-fire  (20 16 20 16 12))
(add g-tank range        ( 4  8 12 10 12))

(define g-wheeled (append gwag gtruck g-car))

(define g-tracked (append ghalf g-spa g-td g-tank))

(define g-vehicle (append g-wheeled g-tracked))

(define g-armored (append ghalf g-car g-spa g-td g-tank))

(define german (append g-arty g-inf g-transport g-car g-spa g-td g-tank))

(add german possible-sides "german")

;;; Non-side-specific type lists.

(define arty (append r-arty g-arty))

(define inf (append r-inf g-inf))

(define engrs (rengrs gengrs))

(define transport (append r-transport g-transport))

(define wheeled (append r-wheeled g-wheeled))

(define tracked (append r-tracked g-tracked))

(define vehicle (append r-vehicle g-vehicle))

(define armored (append r-armored g-armored))

;;; What happens to tanks that get wrecked.

(unit-type wreck (image-name "tank-wrecked") (acp-per-turn 0))

;;; Wrecks don't belong to either side.

(add wreck possible-sides "independent")

(add wreck assign-number false)

;; This should be the end of type definitions.

;;; Static relationships.

(table vanishes-on
  (u* water true)
  (vehicle swamp true)
  )

;; Normal stacking is 4 in a hex.

(add ground capacity 4)

(table unit-size-in-terrain
  (u* solid 1)
  ((block town) ground 4)  ; towns and blocks fill entire hex
  )

;; Towns can have a higher concentration of units.

(add town capacity 8)

;; Trucks, tanks, etc can carry one infantry or artillery unit.

(add vehicle capacity 1)

(table unit-size-as-occupant
  (u* u* 99)  ; can't be occupants usually
  (r-arty r-vehicle 1)
  (r-inf r-vehicle 1)
  (g-arty g-vehicle 1)
  (g-inf g-vehicle 1)
  (arty town 1)
  (inf town 1)
  (vehicle town 1)
)

;;; Vision.

;; No secret towns, although ownership might be unknown.

(add town already-seen 100)

;; Everybody sees everything up to about 2 km or so.
;; (Every unit has *somebody* with binoculars!)

(add u* vision-range 8)
;; No binoculars (or even eyes) for these types though.
(add (block minefield wreck) vision-range -1)

(table see-chance-adjacent
  (u* minefield 50)
  ;; Engineers live to deal with minefields.
  (engrs minefield 100)
  )

(table see-chance
  ;; Can't see minefields from a distance.
  (u* minefield 0)
  )

(table vision-night-effect
  ;; No time to study vague shapes in the dark, so night vision
  ;; greatly curtailed.
  (u* t* 25)
  ;; (should have some types, like CPs, less affected?)
  )

;; Everybody is line-of-sight.

(add u* can-see-behind false)

(table eye-height
  ;; Everybody can at least stand up to look around.
  (u* t* 2)
  ;; One can stand on top of a vehicle to see further.
  (vehicle t* 4)
  )

;; Trees go up to about 20m in height.

(add (woods thick-forest) thickness 20)

;; Unit types are not always easy to distinguish.

(table see-mistake-chance
  (u* r-tank 50.00)
  (u* g-td 50.00)
  (u* g-spa 50.00)
  (u* g-tank 50.00)
  )

(table looks-like
  ;; Types are most likely to look like types in the same class.
  (r-tank r-tank 20)
  (g-spa g-spa 20)
  (g-td g-td 20)
  (g-tank g-tank 20)
  ;; Tracked types are less likely, but still possible.
  (r-tracked r-tracked 10)
  (rhalf u* 0)
  (g-tracked g-tracked 10)
  (ghalf u* 0)
  )

;;; Actions.

(table night-multiplies-acp
  (vehicle t* 50)
  (vehicle road 75)
  )

;;; Movement parameters.

(table mp-to-enter-terrain
  (u* t* 1)
  (u* slope 2)
  ;; While it's more realistic to make inf go slower on slopes too, they
  ;; only have 2 acp to begin with and it's otherwise hard to make the
  ;; numbers work right.  Inf speed doesn't matter much anyway, the battle
  ;; usually moves faster than anybody on foot.
  (inf slope 1)
  (u* road 0)
  (u* water 99)
  (vehicle swamp 99)
  )

(table mp-to-leave-terrain
  (u* t* 1)
  (wheeled gully 20)
  (tracked gully 3)
  (u* road 0)
  )

(table mp-to-traverse
  ;; Roads are the fastest way to get around.
  (u* road 1)
  )

(table mp-to-enter-own
  (u* u* -1)
  (u* wreck 10)
  (u* minefield 20)
  )

(table mp-to-enter-zoc
  (u* u* -1)
  (u* wreck 10)
  (u* minefield 20)
  )

;; Need high cost of entry, takes entire turn, including transport's acp.

(table ferry-on-entry
  ;; Artillery can be loaded/towed by vehicles even though they can't
  ;; move normally.
  (r-vehicle r-arty over-all)
  (g-vehicle g-arty over-all)
  )

;;; Combat parameters.

(table acp-to-attack
  (u* u* 2) ; for now
  ;; Nobody can attack a block or wreck...
  (u* (block wreck) 0)
  ;; ...except for engrs trying to remove one.
  (engrs (block wreck) 1)
  )

(table acp-to-defend
  (u* u* 2) ; for now
  )

;; In general, hp of 2 is healthy, 1 is crippled, 0 is dead or disappeared.

(add u* hp-max 2)
;; Towns are not indestructible, but you'd have to work at it.
(add town hp-max 500)
(add minefield hp-max 1)
;; Trucks and wagons pretty much evaporate in this sort
;; of combat.
(add transport hp-max 1)
;; However, armored equipment can absorb more punishment.
(add armored hp-max 4)
;; ...but not halftracks.
(add (rhalf ghalf) hp-max 2)

(table hit-chance
  ;; By default, don't allow combat.
  (u* u* 0)
  ;; Minefields almost always hit.
  (minefield u* 90)
  ;; But they're unlikely to catch engineers by surprise.
  (minefield engrs 20)
  (r-arty u* 100)
  (r-arty g-inf 50)
  (r-inf  u* 100)
  (r-inf  g-armored 50)
  (r-td   u* 100)
  (r-td   g-inf 50)
  (r-tank u* 100)
  (r-tank g-inf (50 40 50 60))
  (g-arty u* 100)
  (g-inf  u* 100)
  (g-inf  r-tank 10)
  (g-car  u*  70)
  (g-car  r-inf  30)
  (g-spa  u* 100)
  (g-spa  r-inf 50)
  (g-td   u* 100)
  (g-td r-inf 40)
  (g-tank u* 100)
  (g-tank r-inf 40)
  ;; Nobody can do anything to a burning wreck.
  (u* wreck 0)
  ;; Only engineers can clear minefields.
  (u* minefield 0)
  (engrs minefield 20)
  )

(table damage
  (u* u* 0)
  (minefield u* 1)
  (r-arty u* 1)
  (r-inf  u* 1)
  ((rcav rhalf) u* 1)
  (r-td   u* 2)
  (su-152 u* 2)
  (r-tank u* 1)
  (g-arty u* 1)
  (g-inf  u* 1)
  (g-car  u* 1)
  (g-spa  u* 1)
  (g-spa  r-inf 1d2+1)
  (wespe  u* 1d2+2)
  (hummel u* 2d2+2)
  (g-td   u* 1)
  (g-tank u* 1)
  ;; Heavy tanks are more deadly when they connect
  ((js2 tiger1 tiger2) u* 2)
  ;; Nobody can do anything to a burning wreck.
  (u* wreck 0)
  ;; Only engineers can clear minefields.
  (u* minefield 0)
  (engrs minefield 1)
  )

(table occupant-combat
  ;; Artillery can't be used from inside trucks etc.
  (arty vehicle 0)
  ;; (perhaps allow MGs and mortars to fire from halftracks?)
  )

(add arty fire-angle-max 100)
(add g-spa fire-angle-max 75)
(add g-td fire-angle-max 25)
(add g-tank fire-angle-max 25)
(add r-tank fire-angle-max 25)

(table weapon-height
  (u* t* 0)
  (vehicle t* 2)
  )

(table body-height
  (u* t* 2)
  ;; Infantry eats dirt. :-)
  (inf t* 0)
  )

;;; Units eventually bounce back.

(add u* hp-recovery 0.50)

;;; Wrecked armored vehicles continue to be exist.

(add armored wrecked-type wreck)

;;; Towns are easy to capture, that's why they should be protected.

(table capture-chance
  (u* town 100)
  )

;; Minefields clobber any unit trying to pass over them.

(table detonate-on-hit
  ;; One way to get through a minefield is to sacrifice your
  ;; own guys.
  (minefield u* 80)
  )

(table detonate-on-approach-range
  (minefield u* 0)
  )

(add minefield acp-to-detonate 1)

(add minefield hp-per-detonation 0)

(table detonation-damage-at (minefield u* 1d3))

;; All games will have exactly one Russian and one German side.

(set side-library '(
  ((name "Russia") (noun "Russian") (adjective "Russian") (class "russian")
   (color "black,red") (emblem-name "soviet-star") (names-locked true))
  ((name "Germany") (noun "German") (adjective "German") (class "german")
   (color "gray,black") (emblem-name "german-cross") (names-locked true))
  ))

(set sides-min 2)
(set sides-max 2)

;;; The following sets up a simple game, for testing;
;;; usually this module should be used in a more-or-less
;;; preset scenario.

(set alt-blob-height 1000)
(set alt-blob-size 100)
(set alt-smoothing 10)

(add cell-t* alt-percentile-min (  0   5  10   5  90   5))
(add cell-t* alt-percentile-max (  5  10  15  95  95 100))
(add cell-t* wet-percentile-min (  0  50   0  90   0   0))
(add cell-t* wet-percentile-max (100 100  50 100 100 100))

(set edge-terrain clear)

(set country-radius-min 5)

(set country-separation-min 25)
(set country-separation-max 30)

;; An approximation to a Russian tank corps.

(add (r45 r76at) start-with 2)
(add (rrecon rsmg) start-with (2 4))
(add (rtruck rhalf) start-with (6 2))
(add (t34 su-85 su-152) start-with (10 2 2))

;; An approximation to a German battle group.

(add (g75 g88 g20q g120) start-with (2 1 1 1))
(add grifle start-with 3)
(add ghalf start-with 7)
(add (pz4 panther wespe hummel jgdpz4) start-with (3 1 1 1 1))

(table favored-terrain
  ;; Make the less-mobile units start out in transports.
  (arty t* 0)
  (inf t* 0)
  (engrs t* 0)
  )

(table road-chance
  (town town 100)
  )

(table road-into-chance
  (t* t* 100)
  (t* gully 20)
  (gully t* 20)
  (t* water 10)
  (water water 0)
  )

(add town road-to-edge-chance 100)

(set edge-road-density 400)

(add town spur-chance 100)
(add town spur-range 8)

(add u* point-value 1)

(set advantage-max 3)

(scorekeeper (do last-side-wins))

(set shorelines false)

(game-module (instructions "Move fast or die!"))

(game-module (design-notes (
  "Turns represent about 10 minutes each, cells are about 250m across."
  ""
  "make shooting be LOS for armor, specify alts for artillery."
  ""
  )))

