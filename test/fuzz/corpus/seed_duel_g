(game-module "duel"
  (title "The Duel")
  (blurb "Tank duel. Minimalistic submodule for testing.")
  (base-module "tank")
  (variants
   (world-seen true)
   (see-all true)
   (sequential true)
   ("More Tanks" more (true (add u* start-with 1)))
   ) 
)

(area 20 20)

(area (terrain (by-name "+" plains)
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
))

;; Limit to two players only.

(set sides-min 2)
(set sides-max 2)

;; Two sides, no special properties needed (?).

(side 1 (name "One"))

(side 2 (name "Two"))

;; Two tanks, one for each side, at predetermined locations.

(tank 10  7 1)

(tank 10 13 2)

(scorekeeper (when after-event) (do last-side-wins))
