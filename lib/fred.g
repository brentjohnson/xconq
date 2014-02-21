(game-module "fred"
  ;; This is not a standalone game, has only a test setup defined.
  (title "Frederick")
  (version "0.8")
  (blurb "Base module for the Seven Years War.")
  (variants 
   (world-seen true)
   (see-all false)
   (sequential false)
   (world-size (45 30 1000))
   ("Last Side Wins" last-side-wins true
     (true (scorekeeper (do last-side-wins))))
   )
)

; (set see-all true) ; for debugging

(unit-type army (image-name "soldiers"))
(unit-type general (image-name "flag"))
(unit-type marshal (image-name "flag"))
(unit-type prince (image-name "flag"))
(unit-type king (image-name "crown"))
(unit-type depot (image-name "walltown"))
(unit-type town (image-name "town20"))
(unit-type fortress (image-name "fortress"))

(material-type supplies
  (help "generic supplies"))

(terrain-type sea (char "."))
(terrain-type countryside (char "+"))
(terrain-type swamp (char "="))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type river
  (subtype border))
(terrain-type barrier (image-name "mountains")
  (subtype border))
(terrain-type pass (image-name "road")
  (subtype connection))

(define cell-t* (sea countryside swamp forest mountains))
(define leader (general marshal prince king))
(define bases (depot town fortress))

(add sea liquid true)

;;; Static relationships.

(table vanishes-on
  ;; No navy in this game.
  (u* sea true)
  )

;;; Armies carry leaders around and are propelled by them.

(add army capacity 1)
(add bases capacity 10)

(table unit-size-as-occupant
  ;; Disable occupying by default.
  (u* u* 100)
  (army bases 1)
  ;; Army can only have one commander.
  (leader army 1)
  (leader bases 0)
  )

(table unit-capacity-x
  ;; Leaders can carry each other around, higher ranks "carrying" lower.
  (king (general marshal prince) 4)
  (prince (general marshal) 3)
  (marshal (general) 2)
  )

(table unit-size-in-terrain
  (u* t* 0)
  ;; (need to limit armies?)
  )

(table unit-storage-x
  (army supplies 60)
  (bases supplies 600)
  )

;;; Actions.

(add army acp-per-turn 1)
(add leader acp-per-turn 6)
(add bases acp-per-turn 0)

(table occupant-multiplies-acp
  ;; Leaders "get the lead out", accelerate the army.
  (leader army 600)
  )

;;; Movement.

(table mp-to-enter-terrain
  (u* sea 99)
  ;; Crossing rivers is hard for armies.
  (army river 1)
  ;; Mountain barriers are impassable.
  (army barrier 99)
  (leader barrier 99)
)

(table mp-to-traverse
  (army pass 1)
  )

(add army free-mp 1)

;;; Construction.

(add depot cp 6)

(table can-create (army depot 1))

(table can-build (army depot 1))

(table acp-to-create (army depot 1))

(table acp-to-build (army depot 1))

;;; Combat.

(add army hp-max 80)

(table acp-to-attack
  (army u* 3)
  )

(table acp-to-defend
  (army army 3)
  )

(table hit-chance
  (army army 50)
  (army leader 50)
  )

(table damage
  (army army 1d4)
  (army leader 1)
  )

(table withdraw-chance-per-attack
  (army army 25)
  ;; Leaders can usually escape from hits.
  (army leader 90)
  )

(table acp-to-capture
  (army army 1)
  (army leader 1)
  (army bases 1)
  )

(table capture-chance
  ;; implausible for small units capturing large
  (army army 25)
  (army leader 100)
  (army bases 100)
  )

;;; Garrisons prevent immediate capture.

(table protection
  (army fortress 10)
  )

;;; Need a surrender chance for depots vs fortresses.

;;; Backdrop.

(table base-production
  ;; Armies can supply part of needs by foraging.
  (army supplies 15)
  )

(table productivity
  ;; Foraging in the mountains is ineffective.
  (army mountains 20)
  )

(table base-consumption
  (army supplies 60)
  )

(table hp-per-starve
  (army supplies 20.00)
  )

(table out-length
  (bases supplies 5)
  )

(table in-length
  (army supplies 5)
  (bases supplies 5)
  )

;;; Scoring.

(add u* point-value 0)
(add fortress point-value 1)

;;; Seasons.

(world (year-length 26))

(set calendar '(usual week 2))

(set season-names
  ((0 6 "winter") (7 12 "spring") (13 19 "summer") (20 25 "autumn")))

;;; Random generation; should be for testing only.

(add cell-t* alt-percentile-min (  0  70  30  20  90))
(add cell-t* alt-percentile-max ( 20  90  31  90 100))
(add cell-t* wet-percentile-min (  0  50  50   0   0))
(add cell-t* wet-percentile-max (100 100 100 100 100))

(set edge-terrain mountains)

(add u* start-with 1)  ; one of everything

(table favored-terrain
  (u* t* 0)
  (u* countryside 100)
  )

(table unit-initial-supply (u* m* 9999))

;;; Always max out at a 9-month campaign season.

(set last-turn 18)

(set sides-min 2)
(set sides-wanted 4)
(set sides-max 8)

(set side-library '(

((name "Prussia") (adjective "Prussian") (emblem-name "arms-prussia"))
((name "Hannover") (adjective "Hannoverian") (emblem-name "arms-hannover"))
((name "Austria") (adjective "Austrian") (emblem-name "arms-austria"))
((name "Saxony") (adjective "Saxon") (emblem-name "arms-saxony"))
((name "France") (adjective "French") (emblem-name "arms-france"))
((name "Russia") (adjective "Russian") (emblem-name "arms-russia"))
((name "Sweden") (adjective "Swedish") (emblem-name "arms-sweden"))
((name "Empire") (adjective "Empire"))
))

(game-module (notes (
  "This is a game about the Seven Years War in central Europe.  The number of unit types"
  "is small, and there are only a few special characteristics.  The game"
  "is basically one of maneuvering for position, since combat was mostly"
  "ineffective."
  ""
  "Map scale is 15 miles/cell, time is 2 weeks/turn."
  ""
  )))

(game-module (design-notes (
  "A number of items remain to be developed:"
  ""
  "Disable free moves and make combat cost high, to simulate the mutual"
  "consent to combat."
  ""
  "Supply paths (about 4-5 cells)."
  ""
  "River effect on combat."
  )))
