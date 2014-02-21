;; This is a test of routing algorithms.

;;; (should include about 10-20 destinations chosen by variant)
;;; ((implies testing script can choose variants...))
;;; (should include two "passable" type, two "impassable",
;;; 1-2 bords, 1-2 conns)

(include "tank")

(area 30 30)

(area (terrain (by-name "+" plains)
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
  "30+"
))

;; Limit to two players only.

(set sides-min 2)
(set sides-max 2)

;; Two sides, no special properties needed (?).

(side 1)

(side 2)

;;; (no player for one side so tank doesn't move?)

;; Two tanks, one for each side, at predetermined locations.

(tank 10  7 1)

(tank 10 13 2)

;; This motivates the AI to go chasing after the destination.

(scorekeeper (do last-side-wins))
