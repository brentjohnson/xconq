(game-module "napoleon"
  (title "Napoleon")
  (blurb "The Napoleonic wars. Includes experimental weather code.")
  (version "1.0")
  (picture-name "napoleon")
  (variants
   (see-all true)
   (world-seen true)
   (sequential false)
   (world-size (60 30 800))
   ("Clouds" clouds true
    (true
     (add t* clouds-min 0)
     (add t* clouds-max 4)
     ))
   ("Winds" winds true
    (true
     (add t* wind-force-min 1)
     (add t* wind-force-average 1)
     (add t* wind-force-max 4)
     (add t* wind-force-variability 50.00)
     (add t* wind-variability 50.00)
     ))
   ("Last Side Wins" scoring true
     (true
       (scorekeeper (do last-side-wins))
       ))
   )
)

; i g c l t f F B E * @

(unit-type inf (name "infantry") (image-name "soldiers")
  (help "the backbone of the army"))
(unit-type guards (image-name "soldiers")
  (help "the elite of the army"))
(unit-type cav (name "cavalry") (image-name "cavalry")
  (help "zips around for shock effect"))
(unit-type ldr (name "marshal") (image-name "flag")
  (help "controls the movements of land forces"))
(unit-type transport (image-name "brig")
  (help "how armies get across water"))
(unit-type frigate
  (help "the eyes of the fleet"))
(unit-type fleet (image-name "twodecker") (char "F")
  (help "a full line of battle fleet"))
(unit-type balloon (image-name "balloon") (char "B")
  (help "fragile but good for reconnaissance"))
(unit-type E (name "entrenchments") (image-name "camp")
  (help "temporary protection for armies"))
(unit-type city (image-name "town20")  (char "*")
  (help "a typical city"))
(unit-type capital (image-name "city18") (char "@")
  (help "the major city of a country"))

(material-type supply
  (help "abstract combination of food and ammo"))

(terrain-type sea (char ".")
  (help "deep ocean"))
(terrain-type shallows (char ",")
  (help "coastal waters and lakes"))
(terrain-type swamp (char "=")
  (help ""))
(terrain-type desert (char "~")
  (help "dry and sandy terrain"))
(terrain-type plains (char "+")
  (help "open terrain, with farms and pastures"))
(terrain-type forest (char "%")
  (help "deep woods"))
(terrain-type mountains (char "^")
  (help "rugged and mountainous terrain"))
(terrain-type ice (char "_")
  (help "permanent snow and ice, generally impassable"))

(define land-t* (desert plains forest mountains))

(define cell-t* (sea shallows swamp desert plains forest mountains ice))

(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x)
  (help ""))

(terrain-type river (char "<")
  (subtype border) (subtype-x river-x)
  (help ""))

(terrain-type snow (image-name "white")
  (subtype coating)
  (help ""))

(add t* elevation-min -100)
(add t* elevation-max 2000)
(add (sea shallows swamp) elevation-min 0)
(add (sea shallows swamp) elevation-max (0 0 10))
(add (mountains) elevation-min 2000)
(add (mountains) elevation-max 9000)

(area (cell-width 50000))

(add (sea shallows snow) liquid true)

(define water (sea shallows))
(define land (plains desert forest mountains))

(define army-types (inf guards))
(define land-forces (inf guards cav))

(define ships (transport frigate fleet))

(define movers (append land-forces ldr ships balloon))

(define places (E city capital))

;;; Static relationships.

;;; Parts transfer.

(table can-transfer-parts-to false
  (inf inf true)
  (guards guards true)
)

;;; Unit-unit capacities.

(table unit-capacity-x
  ;; Armies can have only one commander.
  ((inf guards) ldr 1)
  ;; (how can leaders carry others as subordinates?)
  )

(add (transport fleet E city capital) capacity (4 2 10 80 80))

(table unit-size-as-occupant
  ;; Disable occupancy normally.
  (u* u* 99)
  (land-forces (transport fleet) 1)
  (ldr u* 1)
  (movers (city capital) 1)
  )

(table occupant-max
  (u* u* 99)
  )

;;; Unit-terrain interaction.

(table vanishes-on
  (land-forces sea true)
  (ships land true)
  (places sea true)
  )

;;; Unit-terrain capacities.

;; Allow effectively infinite capacity everywhere.

(add t* capacity 100)

(table unit-size-in-terrain (u* t* 1))

;;; Unit-material.

(table unit-storage-x
  ;             i   g   c   l   t   f   F   B   E   *   @
  (u* supply (  2   3   2   4  50  50  50   0  10 100 200))
  )

;;; Vision.

;; Although people were tracking weather at this period, reports were
;; neither comprehensive nor timely.

(set see-weather-always false)

;;; In general, secrets don't last long, at least on land.

(add u* already-seen 100)
(add ships already-seen 0)

(add u* spy-chance 100)

;                   i  g  c  l  t  f  F  B  E  *  @
(add u* spy-range ( 8 10  6 10  1  1  1  1 10 10 10))

(table spy-quality (u* u* 80))

;;; Actions.

;                     i  g  c  l  t  f  F  B  E  *  @
(add u* acp-per-turn (1  1  3 10  2  5  3  5  0  1  1))

;; capitals should be more capable?

(table occupant-multiplies-acp
  ;; Leaders enable armies to do things.
  (ldr inf 800)
  (ldr guards 1000)
  )

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (u* t* 1)
  (land-forces sea 99)
  ;; Cavalry is slower in rough terrain.
  (cav (forest mountains) 2)
  ;; Leaders move about as fast as a frigate, when at sea.
  (ldr (sea shallows) 2)
  ;; Leaders are also slower in the mountains.
  (ldr (mountains) 2)
  (ships land 99)
  ;; (no effect for shallows, ships were small then)
  )

(table mp-to-traverse
  (land-forces road 1)
  (ldr road 1)
  )

;;; Construction.

(add u* cp (20 40 8 12 10 20 40 4 1 1 1))

(table acp-to-create
  (army-types E 1)
  ((city capital) movers 1)
  ;; Regular cities may not build balloons
  (city balloon 0)
  )

(table acp-to-build
  ((city capital) movers 1)
  ;; Regular cities may not build balloons
  (city balloon 0)
  )

;;; (ships should need lots of tooling up to build)

;;; Cities can repair anything.

(table acp-to-repair
  ((city capital) u* 1)
  )

;;; Navy ships can repair themselves automatically.

(add (frigate fleet) hp-recovery 1.00)

;;; Combat.

(add u* hp-max (10 10 2 1 3 6 1 1 10 20 40))

(add army-types parts-max 10)

(table acp-to-attack
  ;; These types can defend themselves, but not initiate attacks.
  (ldr u* 0)
  (balloon u* 0)
  (places u* 0)
  )

(table surrender-chance-per-attack
  ;; There's always a chance that a unit will fall into the attacker's
  ;; hands immediately.
  (u* u* 10)
  )

(table withdraw-chance-per-attack
  (u* u* 10)
  )

(table acp-for-retreat
  (army-types army-types 1)
  )

(table hit-chance
  (u* u* 50)
  (guards u* 70)
  ;; Leaders should never expose themselves to combat directly.
  (u* ldr 90)
  (ldr u* 0)
  ;; Balloons are somewhat protected by their altitude.
  (u* balloon 50)
  ;; ...but can't attack anything themselves.
  (balloon u* 0)
  )

(table damage
  (u* u* 1)
  (fleet u* 3)
  )

(table capture-chance
  (army-types city (20 30))
  (army-types capital (10 15))
  )

(table retreat-chance
  (army-types inf 10)
  (army-types guards 5)
  )

(table consumption-per-attack (land-forces supply 1))

(table hit-by (land-forces supply 1))

;; Scuttling and disbanding was easy in those days.

(add movers acp-to-disband 1)

;; Armies can grow and shrink easily.

(add army-types acp-to-transfer-part 1)

;;; Backdrop economy.

(table base-production
  (land-forces supply 1)
  (E supply 1)
  ((city capital) supply 10)
  )

(table productivity
  (land-forces t* 0)
  ;; Foraging only works in settled areas.
  (land-forces plains 100)
  (E desert 0)
  ((city capital) (forest mountains) 70)
  ((city capital) desert 30)
  )

(table base-consumption
  (land-forces supply 1)
  ;; A leader's own consumption is negligible.
  (ships supply 1)
  )

(table out-length
  ;; Armies will not give up any supplies that are in hand.
  (land-forces supply -1)
  ;; Leaders have a staff and supply train that can pass things around.
  (ldr supply 2)
  (places supply 4)
  )

(table in-length
  (land-forces supply 4)
  (ldr supply 2)
  (places supply 4)
  )

(table hp-per-starve
  (land-forces supply 1.00)
  ;; A leader's own consumption is negligible.
  (ships supply 1.00)
  )

(table consumption-as-occupant
  ;; Ships in port just sit there.
  (ships supply 0)
  )

;;; Backdrop environment.

;;; Temperature characteristics of terrain.

(add t* temperature-min -20)
(add water temperature-min 0)
(add desert temperature-min 0)

(add t* temperature-max 30)
(add desert temperature-max 50)
(add mountains temperature-max 20)
(add ice temperature-max 0)

(add t* temperature-average 20)
(add ice temperature-average -10)

(add land temperature-variability 5)

;;; Environmental effects.

(add inf temperature-attrition    '((-30 30) (0 3) (5 0) (30 0) (35 10) (50 50)))
(add guards temperature-attrition '((-30 30) (0 3) (5 0) (30 0) (35 10) (50 50)))
(add cav temperature-attrition    '((-30 30) (0 3) (5 0) (30 0) (35 10) (50 50)))
(add ldr temperature-attrition    '((-30  1) (-20 0)      (40 0)        (50  1)))

(table temperature-protection
  ;; Avoid the cold by staying inside, duh.
  ((city capital) u* true)
  )

;;; The major participants.

(set sides-wanted 6)
(set sides-max 6)

(include "ng-european")
(include "ng-english")
(include "ng-german")
(include "ng-weird")

(add places namer "generic-names")

(set side-library '(
  ((name "France") (adjective "French")(emblem-name "flag-france")
    (unit-namers (places french-place-names)))
  ((name "England") (adjective "English")(emblem-name "flag-uk")
    (unit-namers (places english-place-names)))
  ((name "Spain") (adjective "Spanish")(emblem-name "flag-spain")
    (unit-namers (places spanish-place-names)))
  ((name "Austria") (adjective "Austrian")(emblem-name "arms-austria")
    (unit-namers (places german-place-names)))
  ((name "Prussia") (adjective "Prussian")(emblem-name "arms-prussia")
    (unit-namers (places german-place-names)))
  ((name "Russia") (adjective "Russian")(emblem-name "arms-russia")
    (unit-namers (places russian-place-names)))
  ))

(world 800 (year-length 12))

;;; Set the key points controlling temperature.

(area (temperature-year-cycle (
  ((49 48) (0   0) (1  -5) (3 20) (7 30) (11  5))  ; Paris
  ((86 62) (0 -15) (1 -10) (4 15) (7 20) (11  0))  ; Moscow
  ((56 69) (0   0)                (7 20) (11  0))  ; Stockholm
  ((50 0)  (0  20)                (7 40) (11 20))  ; Sahara
  ((150 0) (0  20)                (7 40) (11 20))  ; Arabia
  )))

(set temperature-moderation-range 1)

(set calendar '(usual month))

(set season-names
  ((0 2 "winter") (3 5 "spring") (6 8 "summer") (9 11 "autumn")))

;; (should be for all types, but list property interp is lame)

(add transport acp-season-effect '((0 25) (3 100) (11 100)))
(add frigate acp-season-effect '((0 25) (3 100) (11 100)))
(add fleet acp-season-effect '((0 25) (3 100) (11 100)))

;; A game's starting units will be full by default.

(table unit-initial-supply (u* m* 9999))

;;; Random setup.  Irrelevant for a historical game, but helpful for testing.

(add cell-t* alt-percentile-min (  0  50  50  51  51  51  95  0))
(add cell-t* alt-percentile-max ( 50  51  51  95  95  95 100  0))
(add cell-t* wet-percentile-min (  0   0  50   0  10  90   0  0))
(add cell-t* wet-percentile-max (100 100 100  10  90 100 100  0))

(add plains country-terrain-min 7)
(set country-radius-min 3)
(set country-separation-min 8)

(add places point-value (0 5 25))

(add (city capital) start-with (5 1))

(table independent-density (city plains 500))

(table favored-terrain add
  (u* t* 0)
  ((city capital) plains 100)
  )

;;; River generation.

;; Rivers are most likely to start in the mountains or forests.

(add (plains forest mountains) river-chance (5.00 8.00 8.00))

;; Rivers empty into lakes if they don't reach the sea.

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (land-t* land-t* 100)
  ;; Try to get a road back out into the plains.
  (cell-t* plains 100)
  ;; Be reluctant to run through hostile terrain.
  (plains (desert forest mountains) (40 30 20))
  )

(set edge-terrain sea)

(game-module (notes "player notes here"
  ))

(game-module (design-notes (
  "Map scale is 50 km/hex, game time is 1 month/turn."
  ""
  "Balloons are more fun than realistic."
  )))

