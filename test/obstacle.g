(game-module "obstacle"
  )

(unit-type army (image-name "soldiers"))

(unit-type wall (image-name "walltown"))

(unit-type pillbox (image-name "airbase"))

(unit-type town (image-name "town20"))

(terrain-type clear)

(add army acp-per-turn 1)

(table hit-chance (army pillbox 50))

(table damage (army pillbox 1))

(table capture-chance (army town 100))

(set synthesis-methods ())

;;; This motivates the AI to go after the town.

(scorekeeper (do last-side-wins))

(add u* point-value 0)
(add town point-value 1)

(set see-all true)

(area 20 15)

(area (terrain
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
  "20a"
))

;; Limit to two players only.

(set sides-min 2)
(set sides-max 2)

;; Two sides, no special properties needed (?).

(side 1)

(side 2)

;;; (no player for one side?)

(town 2 7 1)
(army 3 7 1)

;; Two bases, one for each side.

(town 17 7 2)

;; The obstacles.

(wall 11 12 2)
(wall 12 11 2)
(wall 13 10 2)
(wall 14 9 2)
(wall 14 8 2)
(wall 8 7 2)
(wall 14 7 2)
(wall 15 6 2)
(wall 16 5 2)
(wall 16 4 2)
(wall 16 3 2)
(wall 16 2 2)
(pillbox 8 11 2)
(pillbox 9 10 2)
(pillbox 10 9 2)
(pillbox 10 8 2)
(pillbox 11 8 2)
(pillbox 5 7 2)
(pillbox 11 7 2)
(pillbox 11 6 2)
(pillbox 12 6 2)
(pillbox 12 5 2)
(pillbox 12 4 2)
(pillbox 12 3 2)
